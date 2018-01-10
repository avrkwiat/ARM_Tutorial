#include <stm32f10x_conf.h>
#include "lcd_hd44780_lib.h"


//enum bool {False=0, True=1};
void RCC_Config(void);
void GPIO_Config(void);
void NVIC_Config(void);
void DMA_Config(void);
void ADC_Config(void);

unsigned short int buforADC[2]={0};

int main(void)
{
	volatile unsigned long int i;
	unsigned long int napiecie, temperatura;
	unsigned char Tekst[7] = {"0\0"};
	const unsigned char stopienSymbol[8] = {0x06,0x09,0x09,0x06,0x00,0x00,0x00,0}; //symbol stopnia

	//konfiguracja systemu
	RCC_Config();
	GPIO_Config();
	NVIC_Config();
	DMA_Config();
	ADC_Config();

	/*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
	LCD_Initialize();                                                          //inicjalizacja wysietlacza
	LCD_SetUserChar(1, 1, stopienSymbol);                                      //umiesc symbol stopnia pod kodem =1
	LCD_WriteCommand(HD44780_CLEAR);                                           //wyczysc wyswietlacz
	LCD_WriteText("Pot. : 0,000 V\0");
	LCD_WriteTextXY("Temp.:  0 C\0",0,1);

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);                                    //rozpocznij przetwarzanie AC
	while (1) {
		/*Tu nalezy umiescic glowny kod programu*/
		napiecie = buforADC[0] * 8059/10000;                                     //przelicz wartosc wyrazona jako calkowita, 12-bit na rzeczywista
		sprintf((char *)Tekst, "%d,%03d V\0", napiecie / 1000, napiecie % 1000); //Dzielenie calkowite wyznacza wartosc w V,  dzielenie modulo - czeasc po przecinku
		LCD_WriteTextXY(Tekst, 7, 0);

		temperatura = (1430 - buforADC[1] * 8059/10000)*10/43+25;                //przelicz wartosc wyrazona jako calkowita, 12-bit na rzeczywista, wartosci typowe wg. Datasheet, 5.3.18, str. 75.
		sprintf((char *)Tekst, "%2d C\0", temperatura);
		Tekst[2]=1;                                                              //Wstaw do tekstu znak stopnia
		LCD_WriteTextXY(Tekst, 7, 1);
		for (i=0;i<1500000ul;i++);
	};
	return 0;
}

void NVIC_Config(void)
{
//Konfigurowanie kontrolera przerwan NVIC
#ifdef  VECT_TAB_RAM
// Jezeli tablica wektorow w RAM, to ustaw jej adres na 0x20000000
NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  // VECT_TAB_FLASH
// W przeciwnym wypadku ustaw na 0x08000000
NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}

void RCC_Config(void)
{
	ErrorStatus HSEStartUpStatus;

	// Reset ustawien RCC
	RCC_DeInit();
	// Wlacz HSE
	RCC_HSEConfig(RCC_HSE_ON);
	// Czekaj za HSE bedzie gotowy
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if(HSEStartUpStatus == SUCCESS)
	{
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		// zwloka dla pamieci Flash
		FLASH_SetLatency(FLASH_Latency_2);
		// HCLK = SYSCLK
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		// PCLK2 = HCLK
		RCC_PCLK2Config(RCC_HCLK_Div1);
		// PCLK1 = HCLK/2
		RCC_PCLK1Config(RCC_HCLK_Div2);
		// PLLCLK = 8MHz * 9 = 72 MHz
		RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);
		//ustaw PLLCLK = HSE*7 czyli 8MHz * 7 = 56 MHz - konieczne dla ADC
		//RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_7);
		// Wlacz PLL
		RCC_PLLCmd(ENABLE);
		// Czekaj az PLL poprawnie sie uruchomi
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
		// PLL bedzie zrodlem sygnalu zegarowego
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		// Czekaj az PLL bedzie sygnalem zegarowym systemu
		while(RCC_GetSYSCLKSource() != 0x08);
	}
}


void GPIO_Config(void)
{
	//konfigurowanie portow GPIO
	GPIO_InitTypeDef  GPIO_InitStructure;

	/*Tu nalezy umiescic kod zwiazany z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);//wlacz taktowanie portu GPIO E (LCD)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C (Joystick)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //wlacz taktowanie ADC1
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   //wlacz taktowanie DMA

	/*Tu nalezy umiescic kod zwiazny z konfiguracja poszczegolnych portow GPIO potrzebnych w programie*/

	//port C - Joystick
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//potencjometr
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//wejscie analogowe
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

void DMA_Config(void)
{
	//konfigurowanie DMA

	#define ADC1_DR_Address 0x4001244C;                                             //adres rejestru ADC1->DR

	DMA_InitTypeDef DMA_InitStructure;


	DMA_DeInit(DMA1_Channel1);                                                      //Usun ewentualna poprzednia konfiguracje DMA

	//DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned long int)ADC1_DR_Address;  //Adres docelowy transferu
	DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned long int)&ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (unsigned long int)buforADC;            //Adres poczatku bloku do przeslania
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                              //Kierunek transferu
	DMA_InitStructure.DMA_BufferSize = 2;                                           //Liczba elementow do przeslania (dlugosc bufora)
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                //Wylaczenie automatycznego zwiekszania adresu po stronie ADC
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                         //Wlaczenie automatycznego zwiekszania adresu po stronie pamieci (bufora)
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;     //Rozmiar pojedynczych przesylanych danych po stronie ADC (HalfWord = 16bit)
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;             //Rozmiar pojedynczych przesylanych danych po stronie pamieci (bufora)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                 //Tryb dzialania kontrolera DMA - powtarzanie cykliczne
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;                             //Priorytet DMA - wysoki
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                    //Wylaczenie obslugi transferu z pamieci do pamieci
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);                                    //Zapis konfiguracji

	//Wlacz DMA, kanal 1
	DMA_Cmd(DMA1_Channel1, ENABLE);

}

void ADC_Config(void)
{
	//konfigurowanie przetwornika AC
	ADC_InitTypeDef ADC_InitStructure;

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  //Jeden przetwornik, praca niezalezna
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;                        //Pomiar dwoch kanalow, konieczne skanowanie kanalow
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                  //Pomiar w trybie ciaglym
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;    //Brak wyzwalania zewnetrznego
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                //Wyrownanie danych do prawej - 12 mlodszych bitow znaczacych
	ADC_InitStructure.ADC_NbrOfChannel = 2;                               //Liczba uzywanych kanalow =2
	ADC_Init(ADC1, &ADC_InitStructure);                                 //Incjalizacja przetwornika

	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_1Cycles5);
	//Kanal 8 - GPIOB0 - potencjometr na plytce ZL27ARM
	//Grupa podstawowa, czas probkowania 1,5+12,5=14 cykli = 1us (calkowity czas przetwarania)
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_239Cycles5);
	//Kanal 16 - wewnetrzny czujnik temp. procesora
	//Grupa podstawowa, czas probkowania 239,5+12,5=252 cykli = 18us (calkowity czas przetwarzania)
	//239,5 => 17.1us = czas zalecany w dokumentacji MUC dla miernika temperatury

	ADC_TempSensorVrefintCmd(ENABLE);                                   //Wlaczenie czujnika temperatury w procesorze
	ADC_DMACmd(ADC1,ENABLE);                                            //Wlaczenie DMA dla ADC1

	ADC_Cmd(ADC1, ENABLE);                                                             //Wlacz ADC1

	ADC_ResetCalibration(ADC1);                                            //Reset rejestrow kalibracyjnych ADC1
	while(ADC_GetResetCalibrationStatus(ADC1));                            //Odczekanie na wykonanie resetu
	ADC_StartCalibration(ADC1);                                            //Kalibracja ADC1
	while(ADC_GetCalibrationStatus(ADC1));                                //Odczekanie na zakonczenie kalibracji ADC1
}