#include<msp430.h>

void configureClock() 
{
    BCSCTL1 = CALBC1_1MHZ; // Configurar DCO para 1 MHz (valor de calibração para DCO)
    DCOCTL = CALDCO_1MHZ;
}