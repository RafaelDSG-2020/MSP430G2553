/*
 * =====================================================================================
 *
 *       Filename:  EBYTE.h
 *
 *    Description:  -
 *
 *        Version:  1.0
 *        Created:  11/01/2024 13:43:32
 *       Revision:  none
 *       Compiler:  -
 *
 *         Author:  Isaac Vinicius, isaacvinicius2121@alu.ufc.br
 *   Organization:  UFC-Quixadá
 *
 * =====================================================================================
 */

#ifndef EBYTE_H
#define EBYTE_H

#include <msp430.h>
#include <stdint.h>
#include <string.h>
#define INPUT_PIN 0
#define OUTPUT_PIN 1

#define LOW_PIN 0
#define HIGH_PIN 1
#define INPUT_PIN 0
#define OUTPUT_PIN 1

#define LOW_PIN 0
#define HIGH_PIN 1

#define EBYTE_PIN_M0 BIT2  // P2.2
#define EBYTE_PIN_M1 BIT3  // P2.3
#define EBYTE_AUX_PIN BIT5 // P1.5

#define MODE_NORMAL 0
#define MODE_WAKEUP 1
#define MODE_POWERDOWN 2
#define MODE_PROGRAM 3
#define PIN_RECOVER 50


void UART_init(void);

void UART_send_byte(char byte);

char UART_receive_byte(void);

void send_data(const char* data);

/* Funcoes auxiliares para GPIOs
    Ex. P2.1 como saida: pinMSPMode(&P2DIR, BIT2, OUTPUT_PIN);
*/
void pinMSPMode(volatile unsigned char *port_dir, uint8_t pin, uint8_t mode);

/* Funcoes auxiliares para GPIOs
    Ex. P2.1 HIGH: pinMSPWrite(&P2OUT, BIT2, HIGH_PIN);
*/
void pinMSPWrite(volatile unsigned char *port_out, uint8_t pin, uint8_t level);

/* Funcoes auxiliares para GPIOs
    Ex. P2.1 verifica estado do pino: pinMSPRead(&P2OUT, BIT2);
*/
int pinMSPRead(const volatile unsigned char *port_in, uint8_t pin);
#endif
/*****************************END OF FILE**************************************/