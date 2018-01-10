#include <stm32f10x_conf.h>
#include "lcd_hd44780_lib.h"

void RCC_Config(void);
void GPIO_Config(void);
void NVIC_Config(void);
void ADC_Config(void);

int main(void)
{
	volatile unsigned long int i;
	unsigned long int wartoscADC1  = 0;
	unsigned long int wartoscADC1V = 0;
	unsigned char wartoscADC1VTekst[7] = {"0\0"};

	//konfiguracja systemu
	RCC_Config();
	GPIO_Config();
	NVIC_Config();
	ADC_Config();

	/*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/


	LCD_Initialize();

	LCD_WriteCommand(HD44780_CLEAR);
	LCD_WriteText("Pot. : 0,000 V\0");

	while (1) {
		/*Tu nalezy umiescic glowny kod programu*/
		if (!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9)){ //przycisk ok
			ADC_SoftwareStartConvCmd(ADC1, ENABLE);                       //wyzwolenie pojedynczego pomiaru
			while (!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));           //odczekaj na zakonczenie konwersji
			//wartoscADC1V = ADC_GetConversionValue(ADC1) * 0.8059;   //przelicz wartosc wyrazona jako calkowita, 12-bit na rzeczywista
			wartoscADC1  = ADC_GetConversionValue(ADC1);             //pobiez zmierzona wartosc
			wartoscADC1V =  wartoscADC1 * 8059/10000;                //przelicz wartosc wyrazona jako calkowita, 12-bit na rzeczywista
			sprintf((char *)wartoscADC1VTekst, "%d,%03d V\0", wartoscADC1V / 1000, wartoscADC1V % 1000);//Przekszta³cenie wyniku na tekst, dzielenie calkowite wyzancza wartosc w V, dzielenie modulo - czesc po przecinku
			LCD_WriteTextXY(wartoscADC1VTekst,7,0);
		}
		// Aktualizacja wyswietlacza co okolo 0.25s
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
void ADC_Config(void)
{
	//konfigurowanie przetwornika AC
	ADC_InitTypeDef ADC_InitStructure;

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  //Jeden przetwornik, praca niezalezna
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;                       //Pomiar jednego kanalu, skanowanie kanalow nie potrzebne
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                 //Pomiar w trybie jednokrotnym
	// ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                //Pomiar w trybie ciaglym
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;    //Brak wyzwalania zewnetrznego
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                //Wyrownanie danych do prawej - 12 mlodszych bitow znaczacych
	ADC_InitStructure.ADC_NbrOfChannel = 1;                               //Liczba uzywanych kanalow =1
	ADC_Init(ADC1, &ADC_InitStructure);                                 //Incjalizacja przetwornika

	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_1Cycles5);
	//Kanal 8 - GPIOB0 - potencjometr na plytce ZL29ARM
	//Grupa podstawowa, czas probkowania 1,5+12,5=14 cykli ->  f probkowania = 14MHz/14cykli = 1MHz

	ADC_Cmd(ADC1, ENABLE);                                                          //Wlacz ADC1

	ADC_ResetCalibration(ADC1);                                            //Reset rejestrow kalibracyjnych ADC1
	while(ADC_GetResetCalibrationStatus(ADC1));                         //Odczekanie na wykonanie resetu
	ADC_StartCalibration(ADC1);                                         //Kalibracja ADC1
	while(ADC_GetCalibrationStatus(ADC1));                              //Odczekanie na zakonczenie kalibracji ADC1

}