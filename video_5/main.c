#include <stm32f10x_conf.h>

//enum bool {False=0, True=1};
void RCC_Config(void);
void GPIO_Config(void);
void IWDG_Config(void);
unsigned int i=0;

int main(void)
{
	//konfiguracja systemu
	RCC_Config();
	GPIO_Config();

	GPIO_ResetBits(GPIOE, GPIO_Pin_14 | GPIO_Pin_15); //zapalenie diodek

	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) {//Sprawdzenie, czy system rozpoczyna prace po resecie od IWDG
		GPIO_SetBits(GPIOE, GPIO_Pin_15);                //lewa dioda gaœnie
		RCC_ClearFlag();                                 //wyczysc flage resetu
		for (i=0;i<2000000ul;i++);  //dealy, aby zobaczyæ migniêcie diody
	}

	IWDG_Config();
	/*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/


	while (1) {
		/*Tu nalezy umiescic glowny kod programu*/
		IWDG_ReloadCounter();                            //przeladowanie IWDG
		if (!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9)){ //po wcisneciu SW0 wejscie w nieskonczona petle
		GPIO_SetBits(GPIOE, GPIO_Pin_14); //prawa dioda gaœnie
		while (1);
		}
		GPIO_ResetBits(GPIOE, GPIO_Pin_14 | GPIO_Pin_15); //zapalanie diodek
		for (i=0;i<200000ul;i++);
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


	/*Tu nalezy umiescic kod zwiazany z konfiguracja poszczegolnych portow GPIO potrzebnych w programie*/
	//port B - diody LED
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 |GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);


	//port C - Joystick
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7| GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void IWDG_Config(void){
	//konfigurowanie watchdoga IWDG
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);    // Zezwolenie na zapis rejestrów IWDG
	IWDG_SetPrescaler(IWDG_Prescaler_4);             // IWDG taktowany zegarem: 40kHz/4 = 10kHz
	IWDG_SetReload(0xFFF);                        // Przepelnienie IWDG po okolo 0,4s
	IWDG_ReloadCounter();                           // Przeladowanie IWDG
	IWDG_Enable();                                  // Wlaczenie IWDG i LSI
}