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
    Ex. P2.1 verifica estado do pino: pinMSPRead(P2OUT, BIT2);
*/
int pinMSPRead(const volatile unsigned char *port_in, uint8_t pin);


typedef struct {

// pin variables
    int8_t _M0;
    int8_t _M1;
    int8_t _AUX;

    // variable for the 6 bytes that are sent to the module to program it
    // or bytes received to indicate modules programmed settings
    uint8_t _Params[6];

    // indicidual variables for each of the 6 bytes
    // _Params could be used as the main variable storage, but since some bytes
    // are a collection of several options, let's just make storage consistent
    // also Param[1] is different data depending on the _Save variable
    uint8_t _Save;
    uint8_t _AddressHigh;
    uint8_t _AddressLow;
    uint8_t _Speed;
    uint8_t _Channel;
    uint8_t _Options;
    uint8_t _Attempts;
    
    // individual variables for all the options
    uint8_t _ParityBit;
    uint8_t _UARTDataRate;
    uint8_t _AirDataRate;
    uint8_t _OptionTrans;
    uint8_t _OptionPullup;
    uint8_t _OptionWakeup;
    uint8_t _OptionFEC;
    uint8_t _OptionPower;
    uint16_t _Address;
    uint8_t _Model;
    uint8_t _Version;
    uint8_t _Features;
    uint8_t _buf;

} EBYTE;


// Função SetMode
void SetMode(EBYTE *eb, uint8_t mode);


// Função de inicialização do EBYTE
bool EBYTE_Init(EBYTE *eb, uint8_t _Attempts);


// Função para verificar se há dados disponíveis para leitura na UART
bool UART_available();

// Função para aguardar a conclusão da transmissão de dados UART
void UART_flush();


void EBYTE_SendByte(uint8_t TheByte);
// Função para enviar uma estrutura de dados
bool EBYTE_SendStruct(EBYTE *eb, const void *TheStructure, uint16_t size_);

// Função para receber uma estrutura de dados
bool EBYTE_GetStruct(EBYTE *eb, void *TheStructure, uint16_t size_);

unsigned long millis2();  // Supondo que a função millis() esteja disponível

// Função CompleteTask
void CompleteTask(EBYTE *eb, unsigned long timeout);

// Função Reset
void Reset(EBYTE *eb);


// Funções para configurar valores
void EBYTE_SetSpeed(EBYTE *eb, uint8_t val);

void EBYTE_SetOptions(EBYTE *eb, uint8_t val);

// Funções para obter valores
uint8_t EBYTE_GetSpeed(EBYTE *eb);

uint8_t EBYTE_GetOptions(EBYTE *eb);

void EBYTE_SetAddressH(EBYTE *eb, uint8_t val);

uint8_t EBYTE_GetAddressH(EBYTE *eb);

void EBYTE_SetAddressL(EBYTE *eb, uint8_t val);

uint8_t EBYTE_GetAddressL(EBYTE *eb);


void EBYTE_SetChannel(EBYTE *eb, uint8_t val);

uint8_t EBYTE_GetChannel(EBYTE *eb);

// Funções para configurar e obter a taxa de dados aéreos
void EBYTE_SetAirDataRate(EBYTE *eb, uint8_t val);

uint8_t EBYTE_GetAirDataRate(EBYTE *eb);

// Funções para configurar e obter o bit de paridade
void EBYTE_SetParityBit(EBYTE *eb, uint8_t val);

uint8_t EBYTE_GetParityBit(EBYTE *eb);

// Funções para configurar e obter os modos de opções
void EBYTE_SetTransmissionMode(EBYTE *eb, uint8_t val);

uint8_t EBYTE_GetTransmissionMode(EBYTE *eb);

void EBYTE_SetPullupMode(EBYTE *eb, uint8_t val);

uint8_t EBYTE_GetPullupMode(EBYTE *eb);

void EBYTE_SetWORTIming(EBYTE *eb, uint8_t val);

uint8_t EBYTE_GetWORTIming(EBYTE *eb);

void EBYTE_SetFECMode(EBYTE *eb, uint8_t val);

uint8_t EBYTE_GetFECMode(EBYTE *eb);

void EBYTE_SetTransmitPower(EBYTE *eb, uint8_t val);

uint8_t EBYTE_GetTransmitPower(EBYTE *eb);

// Função para configurar o endereço
void EBYTE_SetAddress(EBYTE *eb, uint16_t val);

// Função para obter o endereço
uint16_t EBYTE_GetAddress(EBYTE *eb);

// Função para configurar a taxa de baud do UART
void EBYTE_SetUARTBaudRate(EBYTE *eb, uint8_t val);

// Função para obter a taxa de baud do UART
uint8_t EBYTE_GetUARTBaudRate(EBYTE *eb);


// Função para construir o byte de velocidade
void EBYTE_BuildSpeedByte(EBYTE *eb);
// Função para construir o byte de opções
void EBYTE_BuildOptionByte(EBYTE *eb);

// Função para obter o estado do pino AUX
bool EBYTE_GetAux(EBYTE *eb);

void EBYTE_SaveParameters(EBYTE *eb, uint8_t val);

// Função para imprimir os parâmetros
void EBYTE_PrintParameters(EBYTE *eb);
// Função ReadParameters
bool ReadParameters(EBYTE *eb);

bool ReadModelData(EBYTE *eb);

// Função para obter o modelo do módulo
uint8_t EBYTE_GetModel(EBYTE *eb);

// Função para obter a versão do módulo
uint8_t EBYTE_GetVersion(EBYTE *eb);

// Função para obter as características do módulo
uint8_t EBYTE_GetFeatures(EBYTE *eb);


void ClearBuffer(void);

#endif
/*****************************END OF FILE**************************************/
