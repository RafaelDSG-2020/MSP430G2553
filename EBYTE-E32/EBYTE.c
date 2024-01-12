/*
 * =====================================================================================
 *
 *       Filename:  EBYTE.c
 *
 *    Description:  -
 *
 *        Version:  1.0
 *        Created:  11/01/2024 13:44:15
 *       Revision:  none
 *       Compiler:  -
 *
 *         Author:  Isaac Vinicius, isaacvinicius2121@alu.ufc.br
 *   Organization:  UFC-Quixadá
 *
 * =====================================================================================
 */
#include "EBYTE.h"

void UART_init(void) {
  // Initialize UART for MSP430 to communicate with E32
  P1SEL = BIT1 + BIT2; // Set P1.1 to RXD and P1.2 to TXD
  P1SEL2 = BIT1 + BIT2;

  UCA0CTL1 |= UCSSEL_2; // Use SMCLK
  UCA0BR0 = 104; // Set baud rate to 9600 with 1MHz clock
  UCA0BR1 = 0;
  UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
}

void UART_send_byte(char byte) {
  while (!(IFG2 & UCA0TXIFG)); // Wait for TX buffer
  UCA0TXBUF = byte; // Send byte
}

char UART_receive_byte(void) {
  while (!(IFG2 & UCA0RXIFG)); // Wait for RX buffer
  return UCA0RXBUF;
}

void send_data(const char* data) {
  while(*data) {
    UART_send_byte(*data++);
  }
}
/* Funcoes auxiliares para GPIOs
    Ex. P2.1 como saida: pinMSPMode(&P2DIR, BIT2, OUTPUT_PIN);
*/
void pinMSPMode(volatile unsigned char *port_dir, uint8_t pin, uint8_t mode) {
  if (mode == OUTPUT_PIN) {
    *port_dir |= pin; // Configura o pino como saída
  } else if (mode == INPUT_PIN) {
    *port_dir &= ~pin; // Configura o pino como entrada
  }
}

/* Funcoes auxiliares para GPIOs
    Ex. P2.1 HIGH: pinMSPWrite(&P2OUT, BIT2, HIGH_PIN);
*/
void pinMSPWrite(volatile unsigned char *port_out, uint8_t pin, uint8_t level) {
  if(level == HIGH_PIN) {
    *port_out |= pin;
  }else if(level == LOW_PIN) {
    *port_out &= ~pin;
  }
}
/* Funcoes auxiliares para GPIOs
    Ex. P2.1 verifica estado do pino: pinMSPRead(&P2OUT, BIT2);
*/
int pinMSPRead(const volatile unsigned char *port_in, uint8_t pin) {
  if ((*port_in & pin)) {
    return 1;  // O pino está em nível alto
  } else {
    return 0;  // O pino está em nível baixo
  }
}