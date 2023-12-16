#include <msp430.h>
#include <stdint.h>

#include "EBYTE.h"

#define PIN_M0 BIT1
#define PIN_M1 BIT2
#define AUX_PIN BIT4 // Assumindo que o pino AUX está conectado ao P1.4

int main(void)
{
  __enable_interrupt(); /* Enable global interrupts. */

  /* Place your initialization/startup code here (e.g. MyInst_Start()) */
  char usbmessage[256];
  __delay_cycles(5000);
  initRadio();
  DATA MyData;
  MyData.Count = 0x01;

  for (;;)
  {
    /* Place your application code here. */

    __delay_cycles(5000);
    EBYTE_SendByte(MyData.Count);
    MyData.Count++;
    __delay_cycles(10000);
  }
}