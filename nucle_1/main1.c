#include "mbed.h"
#include "CAN.h"

Serial pc(D1,D0);                           //serial print message
Serial RS485(D8, D2);                       //RS485_TX RS485_RX
DigitalOut RS485_E(D7);                     //RS485_E

CanTxMsg TxMessage;
uint8_t TransmitMailbox = 0;
int i =0,j=0;

int main() {
        CAN_Config();                       //CAN initiation
        RS485_E = 1;                        //enable RS485 sending status
        
        /* TxMessage */                     //Setting message of Txmessage 
        TxMessage.StdId = 0x10;
        TxMessage.ExtId = 0x1234;
        TxMessage.RTR=CAN_RTR_DATA;
        TxMessage.IDE=CAN_ID_STD;
        TxMessage.DLC=8;
        TxMessage.Data[0] = 'C';
        TxMessage.Data[1] = 'A';
        TxMessage.Data[2] = 'N';
        TxMessage.Data[3] = ' ';
        TxMessage.Data[4] = 'T';
        TxMessage.Data[5] = 'e';
        TxMessage.Data[6] = 's';
        TxMessage.Data[7] = 't';

        pc.printf( "**** This is a RS485_CAN_Shield Send test program ****\r\n");

        while(1) {
                RS485.printf("ncounter=%d ",j);                                         //RS485 sending
                wait(1);  
                TransmitMailbox = CAN_Transmit(CAN1, &TxMessage);                       //CAN sending
        
                i = 0;
                while((CAN_TransmitStatus(CAN1, TransmitMailbox) != CANTXOK) && (i != 0xFFF)){
                        i++;
                }
                if(i == 0xFFF){
                        pc.printf("\r\can send fail\r\n");                              //Send Timeout, fail
                }
                else{
                        pc.printf("\r\nCAN send TxMessage successfully \r\n");          //Send successfully
                }
                pc.printf("\r\nRS485 send: counter=%d\r\n",j++);                        //Print message
                pc.printf("The CAN TxMsg: %s\r\n",TxMessage.Data);
                wait(1);  
        }
}