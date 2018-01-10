#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include "stm32f10x_flash.h"
#include "misc.h"

//enum bool {False=0, True=1};
void RCC_Config(void);
void GPIO_Config(void);


int main(void)
{
	//konfiguracja systemu
	RCC_Config();
	GPIO_Config();
	/*Tu nalezy umiescic ewentualne dalsze funkcje konfigurujace system*/


	while (1) {
		/*Tu nalezy umiescic glowny kod programu*/
		if (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)==0){ //przycis 1
			GPIO_ResetBits(GPIOE, GPIO_Pin_14);
		}
		if (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)==0){ // przycisk 2
			GPIO_SetBits(GPIOE, GPIO_Pin_14);
		}
		 if (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8)==0){// joystic ok
			GPIO_ResetBits(GPIOE, GPIO_Pin_15);
		}
		if (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)==0){//joystic prawo
			GPIO_SetBits(GPIOE, GPIO_Pin_15);
		}

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
	//port E - diody LED
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);


	//port C - Przyciski
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//port C - Joystick
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}