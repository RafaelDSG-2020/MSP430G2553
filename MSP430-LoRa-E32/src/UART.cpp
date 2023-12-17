/*
 * =====================================================================================
 *
 *       Filename:  UART.cpp
 *
 *    Description:  -
 *
 *        Version:  1.0
 *        Created:  16/12/2023 13:57:51
 *       Revision:  none
 *       Compiler:  -
 *
 *         Author:  Isaac Vinicius, isaacvinicius2121@alu.ufc.br
 *   Organization:  UFC-Quixadá
 *
 * =====================================================================================
 */
#include "UART.h"

#define UART_RXD BIT1
#define UART_TXD BIT2


uint8_t UART_1_GetRxBufferSize() {
    // Retorna o número de bytes no buffer de recepção
    return UCA0STAT & UCLISTEN ? 0 : UCA0RXBUF;
}


void UART_1_Start(void ){
    // Configurar pinos RXD e TXD
    P1SEL |= UART_RXD + UART_TXD;
    P1SEL2 |= UART_RXD + UART_TXD;

    // Configurar a taxa de baud rate (ajuste conforme necessário)
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 104;        // 1MHz / 9600
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_1; // Modulation UCBRSx = 1

    // Habilitar módulo UART
    UCA0CTL1 &= ~UCSWRST;
}

void UART_1_PutChar(uint8_t data) {
    // Esperar até que o buffer de transmissão esteja pronto
    while (!(IFG2 & UCA0TXIFG));

    // Enviar o caractere
    UCA0TXBUF = data;
}

void UART_1_ClearRxBuffer(void) {
    // Reseta o flag de interrupção de recepção
    IFG2 &= ~UCA0RXIFG;
}

uint8_t UART_1_GetByte() {
    // Espera até que o buffer de recepção tenha dados
    while (!(IFG2 & UCA0RXIFG));

    // Retorna o byte recebido
    return UCA0RXBUF;
}

void UART_1_PutArray(const void *data, uint8_t size) {
    const uint8_t *data_ptr = (const uint8_t *)data;

    // Envia cada byte da matriz
    for (uint8_t i = 0; i < size; ++i) {
        UART_1_PutChar(data_ptr[i]);
    }
}
