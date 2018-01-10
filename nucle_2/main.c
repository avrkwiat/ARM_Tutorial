#include <stm32f10x_conf.h>

char liczba = 0;

void USART1_IRQHandler(void)
{

    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        liczba = USART_ReceiveData(USART1);
        send_char(liczba);
    }
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {

        liczba = USART_ReceiveData(USART2);
        send_char1(liczba);

    }

}
void delay(int time)
{
    int i;
    for (i = 0; i < time * 4000; i++) {}
}
void send_char(char c)
{
 while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
 USART_SendData(USART2, c);
}

void send_string(const char* s)
{
 while (*s)
 send_char(*s++);
}
void send_char1(char c)
{
 while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
 USART_SendData(USART1, c);
}

void send_string1(const char* s)
{
 while (*s)
 send_char1(*s++);
}

int main(void)
{
    GPIO_InitTypeDef gpio; // obiekt gpio z konfiguracja portow GPIO

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // uruchomienie zegara modulu GPIO

    GPIO_StructInit(&gpio); // domyslna konfiguracja
    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_8; // konfigurujemy pin 5
    gpio.GPIO_Mode = GPIO_Mode_Out_PP; // jako wyjscie
    GPIO_Init(GPIOA, &gpio); // inicjalizacja modulu GPIOA

    USART_InitTypeDef uart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA ,     ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

    GPIO_StructInit(&gpio);
    /* Configure USART1 Tx (PA.09) and UARTT2 Tx (PA.02) as alternate        function push-pull */
    gpio.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_Init(GPIOA, &gpio);

      /* Configure USART1 Rx (PA.10) and USART2 Rx (PA.03) as input floating */
      gpio.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_3;
      gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
      GPIO_Init(GPIOA, &gpio);

    USART_StructInit(&uart);
    uart.USART_BaudRate = 9600;
    USART_Init(USART2, &uart);

    USART_StructInit(&uart);
    uart.USART_BaudRate = 19200;
    USART_Init(USART1, &uart);

    USART_Cmd(USART2, ENABLE);
    USART_Cmd(USART1, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;

    #ifdef  VECT_TAB_RAM
    // Jezeli tablica wektorow w RAM, to ustaw jej adres na 0x20000000
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
    #else  // VECT_TAB_FLASH
    // W przeciwnym wypadku ustaw na 0x08000000
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
    #endif

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    //Przerwanie od USART4
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //Przerwanie od USART2
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //za³¹czenie przerwania RX
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); //za³¹czenie przerwania RX

    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_EnableIRQ(USART1_IRQn);

    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    //GPIO_SetBits(GPIOA, GPIO_Pin_8);

    while (1) {
        GPIO_SetBits(GPIOA, GPIO_Pin_5); // zapalenie diody
        delay(100);
        GPIO_ResetBits(GPIOA, GPIO_Pin_5); // zgaszenie diody
        delay(400);

       // send_string("Hello world!\r\n = ");
        //send_string1("Hello world1111!\r\n = ");

    }
}