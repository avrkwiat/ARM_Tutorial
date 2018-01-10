#include "mbed.h"

void com_Rx_interrupt();
void RS485_Rx_interrupt();

Serial RS485(D8, D2, 19200); //RS485_TX RS485_RX
Serial pc(D1,D0); //serial print message

DigitalOut RS485_E(D7); //RS485_E

DigitalOut myled(LED1);

int main()
{
    pc.attach(&com_Rx_interrupt, Serial::RxIrq);
    NVIC_EnableIRQ(USART2_IRQn);
    
    RS485_E = 1;//enable RS485 sending status  
    RS485.attach(&RS485_Rx_interrupt, Serial::RxIrq);
    NVIC_EnableIRQ(USART1_IRQn);
    
    pc.printf("Hello World !\n");
    while(1) {
        wait(1);
       // pc.printf("This program runs since %d seconds.\n");
       // RS485.printf("uart");
        myled = !myled;
    }
}

/////////////przerwania//////////

void com_Rx_interrupt() {
    RS485.printf("%c",pc.getc());
    return;
}
void RS485_Rx_interrupt() {
    pc.printf("%c",RS485.getc()); 
    return;
}