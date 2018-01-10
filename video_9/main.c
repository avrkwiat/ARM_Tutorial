#include "stm32f10x.h"

void SysTick_Handler(void)  {
GPIO_WriteBit(GPIOE, GPIO_Pin_15, (BitAction) ((1-GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_15))));
}

int main (void)  {
uint32_t returnCode;
returnCode = SysTick_Config(72000000/ 1000);    //500Hz T=2ms
  if (returnCode != 0)  {                                   /* Check return code for errors */
    // Error Handling
  }
  //konfigurowanie portow GPIO
  GPIO_InitTypeDef  GPIO_InitStructure;

  /*Tu nalezy umiescic kod zwiazany z konfiguracja sygnalow zegarowych potrzebnych w programie peryferiow*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);//wlacz taktowanie portu GPIO E
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlacz taktowanie portu GPIO C

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  while(1);
}