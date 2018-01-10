#include "stm32f10x.h"
 
void RCC_Config(void);
void GPIO_Config(void);
void NVIC_Config(void);
 
int main(void)
{
	//konfiguracja systemu
	RCC_Config();
	GPIO_Config();
	NVIC_Config();
	/*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/
	 
	GPIO_SetBits(GPIOE, GPIO_Pin_14 | GPIO_Pin_15); //wy³¹czenie diodek
	 
	while (1) {
		/*Tu nalezy umiescic glowny kod programu*/
	 
	};
	return 0;
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
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);//wlacz taktowanie portu GPIO E
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//wlacz taktowanie funkcji alternatywnych
	 
	/*Tu nalezy umiescic kod zwiazany z konfiguracja poszczegolnych portow GPIO potrzebnych w programie*/
	//port B - diody LED
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	 
	//port C - Joystick
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
 
void NVIC_Config(void)
{
	//Konfigurowanie kontrolera przerwan NVIC
	 
	NVIC_InitTypeDef  NVIC_InitStructure;
	EXTI_InitTypeDef  EXTI_InitStructure;
	 
	#ifdef  VECT_TAB_RAM
	// Jezeli tablica wektorow w RAM, to ustaw jej adres na 0x20000000
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
	#else  // VECT_TAB_FLASH
	// W przeciwnym wypadku ustaw na 0x08000000
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	#endif
	 
	//Konfiguracja NVIC - ustawienia priorytetow przerwania EXTI9_5 Dla przycisku OK
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);                //Wybor modelu grupowania przerwan
	 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;               //Wybor konfigurowanego IRQ
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;      //Priorytet grupowy
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;             //Podpriorytet
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                //Wlaczenie obslugi IRQ
	NVIC_Init(&NVIC_InitStructure);
	 
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource9);    //Ustawienie zrodla przerwania
	 
	//Konfiguracja przerwania EXTI0 na linie 0
	EXTI_InitStructure.EXTI_Line = EXTI_Line9;                     //Wybor linii
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;            //Ustawienie generowania przerwania (a nie zdarzenia)
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;        //Wyzwalanie zboczem opadajacym (wcisniecie przycisku)
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                      //Wlaczenie przerwania
	EXTI_Init(&EXTI_InitStructure);
	 
	//Konfiguracja NVIC - ustawienia priorytetow przerwania EXTI9_5 Dla przycisku UP
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);                //Wybor modelu grupowania przerwan
	 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;               //Wybor konfigurowanego IRQ
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;      //Priorytet grupowy
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;             //Podpriorytet
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                //Wlaczenie obslugi IRQ
	NVIC_Init(&NVIC_InitStructure);
	 
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);    //Ustawienie zrodla przerwania
	 
	//Konfiguracja przerwania EXTI0 na linie 0
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;                     //Wybor linii
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;            //Ustawienie generowania przerwania (a nie zdarzenia)
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;        //Wyzwalanie zboczem opadajacym (wcisniecie przycisku)
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                      //Wlaczenie przerwania
	EXTI_Init(&EXTI_InitStructure);
	 
}