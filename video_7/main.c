#include <stm32f10x_conf.h>
#include "lcd_hd44780_lib.h"

void RCC_Config(void);
void GPIO_Config(void);
void NVIC_Config(void);
void USART_Configuration();
void USART_PutChar(char value);
char USART_GetChar(void);
void USART_PutString(char * s);
void UART4_IRQHandler(void)
{
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		LCD_WriteData(USART_ReceiveData(UART4));
	}
	/*
	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
	USART_SendData(UART4, danedowyslania);      //Wyslij kolejny znak i zwieksz indeks bufora wyjsciowego

	}*/
}

int main(void)
{
	//      volatile unsigned long int i;
	char stanDiod;;
	//konfiguracja systemu
	char Message[18];
	RCC_Config();
	GPIO_Config();
	NVIC_Config();
	USART_Configuration();
	// USART_Config();

	/*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
	LCD_Initialize();                                                          //inicjalizacja wysietlacza
	LCD_WriteCommand(HD44780_CLEAR);                                           //wyczysc wyswietlacz
	USART_PutString("Udalo sie, RS232 dziala jak nalezy!!\0");
	while (1) {
		/*Tu nalezy umiescic glowny kod programu*/

		if (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)==0){ //przycis 1
			GPIO_ResetBits(GPIOE, GPIO_Pin_14);
			USART_PutString("Tu Marcin Kwiatkowski Pokazujê wam teraz RS232\0");
			for (int i=0;i<1500000ul;i++);
		}


	};
	return 0;
}

void NVIC_Config(void)
{
	//Konfigurowanie kontrolera przerwan NVIC
	NVIC_InitTypeDef NVIC_InitStructure;

	#ifdef  VECT_TAB_RAM
	// Jezeli tablica wektorow w RAM, to ustaw jej adres na 0x20000000
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
	#else  // VECT_TAB_FLASH
	// W przeciwnym wypadku ustaw na 0x08000000
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	#endif

	//Wybranie grupy priorytetów
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	//Przerwanie od USART1
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
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
		// RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_7);
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
	// RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);//wlacz taktowanie UART4
	/*Tu nalezy umiescic kod zwiazny z konfiguracja poszczegolnych portow GPIO potrzebnych w programie*/
	//diody
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	//port C - Joystick
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 |GPIO_Pin_13 |GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
void USART_Configuration()
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART4, ENABLE);
	//PC10 - Tx
	GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed           = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//PC11 - Rx
	GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_11;
	//    GPIO_InitStructure.GPIO_Speed           = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 9600;                                      //Predkosc transmisji =9600bps
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     //Dlugosc slowa = 8 bitow
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                          //1 bit stopu
	USART_InitStructure.USART_Parity = USART_Parity_No;                             //Brak kontroli parzystosci
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //Kontrola przeplywu danych (brak)
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                    //Tryb pracy
	USART_Init(UART4, &USART_InitStructure);

	USART_Init(UART4, &USART_InitStructure);

	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE); //za³¹czenie przerwania RX
	USART_Cmd(UART4, ENABLE);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void USART_PutChar(char value)
{
	USART_SendData(UART4, value);
	while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
char USART_GetChar(void)
{
	while(USART_GetFlagStatus(UART4, USART_FLAG_RXNE) == RESET);
	USART_ReceiveData(UART4);
}
//-----------------------------------------------------------------------------
// Send string
//-----------------------------------------------------------------------------
void USART_PutString(char * s)
{
	while(*s)
	USART_PutChar(*s++);
}

//-----------------------------------------------------------------------------
// End of file
//---------------