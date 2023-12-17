#include <msp430.h>
#include <stdint.h>

#include "EBYTE.h"
#include "UART.h"

typedef struct data_t
{
  // data sent and received
  uint8_t Count;
  uint8_t message1;
  uint8_t message2;
  uint8_t message3[6];
} DATA;

int main(void)
{
  char usbmessage[256];
  initRadio();
  DATA MyData;

  while (1) {
    /* Place your application code here. */

    MyData.message1 = 0xBA;
    MyData.message2 = 0x15;
    MyData.message3[0] = 'H';MyData.message3[1] = 'e'; MyData.message3[2] = 'l';
    MyData.message3[3] = 'l'; MyData.message3[4] = 'o'; MyData.message3[5] = '\0';   

    __delay_cycles(5000);
    EBYTE_SendByte(MyData.Count);
    EBYTE_SendByte(MyData.message1);
    EBYTE_SendByte(MyData.message2);
    //        EBYTE_SendByte(MyData.message3[0]);
    //        EBYTE_SendByte(MyData.message3[1]);
    //        EBYTE_SendByte(MyData.message3[2]);
    //        EBYTE_SendByte(MyData.message3[3]);
    //        EBYTE_SendByte(MyData.message3[4]);
    //        EBYTE_SendByte(MyData.message3[5]);
    UART_1_PutArray((void *)MyData.message3, 6);


    //sprintf(usbmessage,"Sending: %d\n", MyData.Count);
    //sendUsb((void *)usbmessage, strlen(usbmessage));
    MyData.Count++;
    __delay_cycles(10000);
    
  }
}