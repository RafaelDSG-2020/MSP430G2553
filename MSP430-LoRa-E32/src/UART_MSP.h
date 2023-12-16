/*
 * =====================================================================================
 *
 *       Filename:  UART_MSP.h
 *
 *    Description:  -
 *
 *        Version:  1.0
 *        Created:  16/12/2023 13:37:53
 *       Revision:  none
 *       Compiler:  -
 *
 *         Author:  Isaac Vinicius, isaacvinicius2121@alu.ufc.br
 *   Organization:  UFC-Quixadá
 *
 * =====================================================================================
 */

#ifndef UART_MSP_H 
#define UART_MSP_H


#include <msp430.h>

#include <stdint.h>

#include <stdio.h>

#include "UART_MSP.h"

uint8_t UART_1_GetRxBufferSize();
void UART_1_ClearRxBuffer();
void UART_1_Start();
void UART_1_PutChar(uint8_t data);
uint8_t UART_1_GetByte();


#endif
/*****************************END OF FILE**************************************/