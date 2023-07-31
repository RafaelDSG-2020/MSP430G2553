//#include "UART.h"
#include <msp430.h>
#include <stdio.h> // Incluir esta biblioteca para usar sprintf()

void init_uart()
{
    // WDTCTL = WDTPW | WDTHOLD;
    // BCSCTL1 = CALBC1_1MHZ;
    // DCOCTL = CALDCO_1MHZ;

    P1SEL = BIT1 | BIT2;
    P1SEL2 = BIT1 | BIT2;

    UCA0CTL1 |= UCSWRST + UCSSEL_2;
    UCA0BR0 = 52;  // settings for 19200 baud
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_0;
    UCA0CTL1 &= ~UCSWRST;
}

void ser_output(char *str)
{
    while (*str != 0)
    {
        while (!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = *str++;
       // __delay_cycles(100000/2);
    }
}
