#include <msp430.h>


void configureClock();
void configureTimerA();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Parar o Watchdog Timer

    P1DIR |= BIT0; // Configurar o P1.0 como saída (LED)

    configureClock(); // Configurar o clock do sistema para 32 kHz
    configureTimerA(); // Configurar o Timer A com SMCLK

    __enable_interrupt(); // Habilitar as interrupções globais

    while (1) 
    {
        // Loop principal
    }
}

void configureClock() 
{
    BCSCTL1 = CALBC1_1MHZ; // Configurar DCO para 1 MHz (valor de calibração para DCO)
    DCOCTL = CALDCO_1MHZ;
}

void configureTimerA() {
    // Configurar o Timer A com SMCLK (TASSEL_2), sem divisão de clock (ID_0) e modo de contagem ascendente (MC_1)
    TA0CTL = TASSEL_2 + ID_3 + MC_1;

    // Definir o valor do contador para gerar uma interrupção a cada 32 kHz / 32768 = 1 Hz = 1 segundo
    TA0CCR0 = (62500); /// Valor de TIMER de 1 Segundo.

    TA0CCTL0 |= CCIE; // Habilitar a interrupção de comparação do canal 0 do Timer A
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TimerA_ISR(void) {
    static unsigned int interruptCount = 0;

    if (interruptCount == 1) { // Realiza a ação na segunda interrupção (após 2 estouros)
        P1OUT ^= BIT0; // Alternar o estado do LED (P1.0)
        interruptCount = 0; // Reiniciar o contador de interrupções
    } else {
        interruptCount++; // Incrementar o contador de interrupções
    }
}
// void configureTimerA();

// int main(void) {
//     WDTCTL = WDTPW | WDTHOLD; // Parar o Watchdog Timer

//     P1DIR |= BIT0; // Configurar o P1.0 como saída (LED)

//     configureTimerA(); // Configurar o Timer A

//     __enable_interrupt(); // Habilitar as interrupções globais

//     while (1) {
//         // Loop principal
//     }
// }

// void configureTimerA() {
//     TA0CTL = TASSEL_2 + ID_3 + MC_1; // Configurar o Timer A com SMCLK (TASSEL_2), Prescaler de divisão por 8 (ID_3) e modo de contagem ascendente (MC_1)
//     TA0CCR0 = 62500; // Definir o valor do contador para gerar uma interrupção a cada 1 segundo (1 MHz / 8 / 12500 = 1 Hz = 1 segundo)
//     TA0CCTL0 |= CCIE; // Habilitar a interrupção de comparação do canal 0 do Timer A
// }

// #pragma vector=TIMER0_A0_VECTOR
// __interrupt void TimerA_ISR(void) {
//     // Rotina de interrupção do Timer A (executada a cada 1 segundo)
//     P1OUT ^= BIT0; // Alternar o estado do LED (P1.0)
// }



// int main(void)
// {
//   WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
//   P1DIR |= 0x01;                            // P1.0 output
//   CCTL0 = CCIE;                             // CCR0 interrupt enabled
//   CCR0 = 50000;
//   TACTL = TASSEL_2 + MC_2;                  // SMCLK, contmode

//   __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
// }

// // Timer A0 interrupt service routine
// #if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
// #pragma vector=TIMER0_A0_VECTOR
// __interrupt void Timer_A (void)
// #elif defined(__GNUC__)
// void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
// #else
// #error Compiler not supported!
// #endif
// {
//   P1OUT ^= 0x01;                            // Toggle P1.0
//   CCR0 += 50000;                            // Add Offset to CCR0
// }
