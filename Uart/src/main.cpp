#include <msp430.h>
#include <stdio.h> // Incluir esta biblioteca para usar sprintf()

int i;
long int y = -120000;
//int y = 30000;
// char text[] = "I am an MSP430\r\n";
void ser_output(char *str);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1SEL = BIT1 | BIT2;
    P1SEL2 = BIT1 | BIT2;

    UCA0CTL1 |= UCSWRST + UCSSEL_2;
    UCA0BR0 = 52;  // settings for 19200 baud
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_0;
    UCA0CTL1 &= ~UCSWRST;
    

    while (1)
    {
        char buffer[100]; // Buffer para armazenar a string com o valor de y
        sprintf(buffer, "%ld\r\n", y); // Converter y para string
       // sprintf(buffer, "%d\r\n", x); // Converter y para string

        ser_output(buffer); // Enviar o valor de y pela UART

        y--; // Incrementar y
    }
}

void ser_output(char *str)
{
    while (*str != 0)
    {
        while (!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = *str++;
		//__delay_cycles(100000);
    }
}
