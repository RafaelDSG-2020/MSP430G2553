#include<msp430.h>
#include <stdio.h>
#include<CLOCK/configClock.h>
#include<HX711/HX711.h>
#include<UART/UART.h>

long int y = 0;

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD; // Parar o Watchdog Timer
  configureClock(); 
  init_uart();
  hx711_init();
 
  
  while (1)
  {
       y = hx711_read();
       //y= 10;
        char buffer[100]; // Buffer para armazenar a string com o valor de y
        sprintf(buffer, "%ld\r\n", y); // Converter y para string
       // sprintf(buffer, "%d\r\n", x); // Converter y para string

        ser_output(buffer); // Enviar o valor de y pela UART

        __delay_cycles(10000);

       // y++; // Incrementar y
  }
  

}
