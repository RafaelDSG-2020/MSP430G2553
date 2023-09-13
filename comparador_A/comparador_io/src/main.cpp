#include <msp430.h>
// https://electronics.stackexchange.com/questions/162839/where-am-i-going-wrong-with-the-comparator-in-msp430

/* Este programa é destinado à placa de desenvolvimento Launchpad MSP430.
 * Um sensor está conectado de Vcc a Gnd, com o pino I/O conectado à entrada CA1 (P1.1).
 * Quando a tensão no sensor estiver abaixo do limite, o LED verde deve acender.
 * Quando a tensão exceder o limite, o LED vermelho deve acender.
 *
 * Para esta configuração, Vcc = 3,53 V
 * Referência = Vcc/2 = 1,77 V
 *       Vin = 1,76 V ou superior: LED verde aceso?
 *       Vin = 1,70 V ou inferior: LED vermelho aceso?
 */

#define LED_GREEN    BIT6
#define LED_RED      BIT0
#define SENSOR       BIT7

int main(void) {
  WDTCTL = WDTPW + WDTHOLD;         // Desativa o watchdog

  // Configura os pinos como saídas: P1.0, P1.6 e P1.7
  P1DIR = LED_GREEN + LED_RED + SENSOR;

  // Configura o pino P1.1 (CA1) para o modo de entrada analógica
  CACTL2 = P2CA4;

  // Configura o comparador analógico:
  // - Seleciona a referência como 0,5*Vcc
  // - Habilita o comparador
  CACTL1 = CARSEL + CAREF_2 + CAON;

  // Configura o pino P1.7 para atualizar com base na saída do comparador (CAOUT)
  P1SEL = SENSOR;

  unsigned int led_mask = 0;

  while (1) {
    
    led_mask = P1OUT;
    // Verifica o resultado da comparação no registrador CACTL2
    if (CACTL2 & 0x01){
      led_mask &= ~LED_GREEN; // LED verde desligado
      led_mask |= LED_RED;    // LED vermelho aceso
    }else{
      led_mask &= ~LED_RED;  // LED vermelho desligado
      led_mask |= LED_GREEN; // LED verde aceso
    }

    // Atualiza os LEDs de acordo com o resultado da comparação
    P1OUT = led_mask;
  }
}
