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

#include <msp430.h>
#include <stdint.h>
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

void send_data(const char * data) {
  while ( * data) {
    UART_send_byte( * data++);
  }
}
/* Funcoes auxiliares para GPIOs
    Ex. P2.1 como saida: pinMSPMode(P2DIR, BIT2, OUTPUT_PIN);
*/
void pinMSPMode(volatile unsigned char *port_dir, uint8_t pin, uint8_t mode) {
  if (mode == OUTPUT_PIN) {
    *port_dir |= pin; // Configura o pino como saída
  } else if (mode == INPUT_PIN) {
    *port_dir &= ~pin; // Configura o pino como entrada
  }
}

/* Funcoes auxiliares para GPIOs
    Ex. P2.1 HIGH: pinMSPWrite(P2OUT, BIT2, HIGH_PIN);
*/
void pinMSPWrite(volatile unsigned char *port_out, uint8_t pin, uint8_t level) {
  if(level == HIGH_PIN) {
    *port_out |= pin;
  }else if(level == LOW_PIN) {
    *port_out &= ~pin;
  }
}
/* Funcoes auxiliares para GPIOs
    Ex. P2.1 verifica estado do pino: pinMSPRead(P2OUT, BIT2);
*/
int pinMSPRead(const volatile unsigned char *port_in, uint8_t pin) {
  if ((*port_in & pin)) {
    return 1;  // O pino está em nível alto
  } else {
    return 0;  // O pino está em nível baixo
  }
}



unsigned long millis2() {
  __delay_cycles(50000);
}

// Função SetMode
void SetMode(EBYTE * eb, uint8_t mode) {
  __delay_cycles(PIN_RECOVER);

  switch (mode) {
  case MODE_NORMAL:
    pinMSPWrite(&P2OUT, eb -> _M0, LOW_PIN);
    pinMSPWrite(&P2OUT, eb -> _M1, LOW_PIN);
    break;
  case MODE_WAKEUP:
    pinMSPWrite(&P2OUT, eb -> _M0, HIGH_PIN);
    pinMSPWrite(&P2OUT, eb -> _M1, LOW_PIN);
    break;
  case MODE_POWERDOWN:
    pinMSPWrite(&P2OUT, eb -> _M0, LOW_PIN);
    pinMSPWrite(&P2OUT, eb -> _M1, HIGH_PIN);
    break;
  case MODE_PROGRAM:
    pinMSPWrite(&P2OUT, eb -> _M0, HIGH_PIN);
    pinMSPWrite(&P2OUT, eb -> _M1, HIGH_PIN);
    break;
  }

  __delay_cycles(PIN_RECOVER);
  CompleteTask(eb, 4000);
}
// Função de inicialização do EBYTE
bool EBYTE_Init(EBYTE * eb, uint8_t _Attempts) {
  eb -> _M0 = EBYTE_PIN_M0;
  eb -> _M1 = EBYTE_PIN_M1;
  eb -> _AUX = EBYTE_AUX_PIN;

  pinMSPMode(&P2DIR, eb -> _M0, OUTPUT_PIN); // Configura M0 como saída
  pinMSPMode(&P2DIR, eb -> _M1, OUTPUT_PIN); // Configura M1 como saída
  pinMSPMode(&P1DIR, eb -> _AUX, INPUT_PIN); // Configura AUX como entrada

  __delay_cycles(1000);

  if (_Attempts < 1) _Attempts = 1;
  if (_Attempts > 10) _Attempts = 10;

  SetMode(eb, MODE_NORMAL); // Definir modo para Normal

  // Ler os dados do modelo e parâmetros
  bool ok = ReadModelData(eb);
  if (!ok) return false;

  ok = ReadParameters(eb);
  if (!ok) return false;

  return true;
}

// Função para verificar se há dados disponíveis para leitura na UART
bool UART_available() {
  return (IFG2 & UCA0RXIFG) != 0; // Retorna verdadeiro se o buffer de recepção contém dados
}

// Função para aguardar a conclusão da transmissão de dados UART
void UART_flush() {
  while (!(IFG2 & UCA0TXIFG)); // Espera até que o buffer de transmissão esteja vazio
}

void EBYTE_SendByte(uint8_t TheByte) {
  UART_send_byte(TheByte);
}

uint8_t EBYTE_GetByte() {
  return UART_receive_byte();
}

// Função para enviar uma estrutura de dados
bool EBYTE_SendStruct(EBYTE * eb,
  const void * TheStructure, uint16_t size_) {
  uint16_t i;
  const uint8_t * p = (const uint8_t * ) TheStructure;

  for (i = 0; i < size_; ++i) {
    UART_send_byte(p[i]);
  }

  CompleteTask(eb, 1000);

  return (i == size_);
}

// Função para receber uma estrutura de dados
bool EBYTE_GetStruct(EBYTE * eb, void * TheStructure, uint16_t size_) {
  uint16_t i;
  uint8_t * p = (uint8_t * ) TheStructure;

  for (i = 0; i < size_; ++i) {
    p[i] = UART_receive_byte();
  }

  CompleteTask(eb, 1000);

  return (i == size_);
}

// Função CompleteTask
void CompleteTask(EBYTE * eb, unsigned long timeout) {
  unsigned long t = millis2();

  // Verificar o limite do unsigned long
  if (((unsigned long)(t + timeout)) == 0) {
    t = 0;
  }

  if (eb -> _AUX != -1) {
    while (pinMSPRead(&P1IN, eb -> _AUX) == LOW_PIN) {
      __delay_cycles(PIN_RECOVER);
      if ((millis2() - t) > timeout) {
        break;
      }
    }
  } else {
    // Alternativa se não houver pino AUX
    __delay_cycles(100000);
  }
}

// Função Reset
void Reset(EBYTE * eb) {
  SetMode(eb, MODE_PROGRAM);

  UART_send_byte(0xC4);
  UART_send_byte(0xC4);
  UART_send_byte(0xC4);

  CompleteTask(eb, 4000);

  SetMode(eb, MODE_NORMAL);
}

// Funções para configurar valores
void EBYTE_SetSpeed(EBYTE * eb, uint8_t val) {
  eb -> _Speed = val;
}

void EBYTE_SetOptions(EBYTE * eb, uint8_t val) {
  eb -> _Options = val;
}

// Funções para obter valores
uint8_t EBYTE_GetSpeed(EBYTE * eb) {
  return eb -> _Speed;
}

uint8_t EBYTE_GetOptions(EBYTE * eb) {
  return eb -> _Options;
}

void EBYTE_SetAddressH(EBYTE * eb, uint8_t val) {
  eb -> _AddressHigh = val;
}

uint8_t EBYTE_GetAddressH(EBYTE * eb) {
  return eb -> _AddressHigh;
}

void EBYTE_SetAddressL(EBYTE * eb, uint8_t val) {
  eb -> _AddressLow = val;
}

uint8_t EBYTE_GetAddressL(EBYTE * eb) {
  return eb -> _AddressLow;
}

void EBYTE_SetChannel(EBYTE * eb, uint8_t val) {
  eb -> _Channel = val;
}

uint8_t EBYTE_GetChannel(EBYTE * eb) {
  return eb -> _Channel;
}
// Funções para configurar e obter a taxa de dados aéreos
void EBYTE_SetAirDataRate(EBYTE * eb, uint8_t val) {
  eb -> _AirDataRate = val;
  EBYTE_BuildSpeedByte(eb);
}

uint8_t EBYTE_GetAirDataRate(EBYTE * eb) {
  return eb -> _AirDataRate;
}

// Funções para configurar e obter o bit de paridade
void EBYTE_SetParityBit(EBYTE * eb, uint8_t val) {
  eb -> _ParityBit = val;
  EBYTE_BuildSpeedByte(eb);
}

uint8_t EBYTE_GetParityBit(EBYTE * eb) {
  return eb -> _ParityBit;
}

// Funções para configurar e obter os modos de opções
void EBYTE_SetTransmissionMode(EBYTE * eb, uint8_t val) {
  eb -> _OptionTrans = val;
  EBYTE_BuildOptionByte(eb);
}

uint8_t EBYTE_GetTransmissionMode(EBYTE * eb) {
  return eb -> _OptionTrans;
}

void EBYTE_SetPullupMode(EBYTE * eb, uint8_t val) {
  eb -> _OptionPullup = val;
  EBYTE_BuildOptionByte(eb);
}

uint8_t EBYTE_GetPullupMode(EBYTE * eb) {
  return eb -> _OptionPullup;
}

void EBYTE_SetWORTIming(EBYTE * eb, uint8_t val) {
  eb -> _OptionWakeup = val;
  EBYTE_BuildOptionByte(eb);
}

uint8_t EBYTE_GetWORTIming(EBYTE * eb) {
  return eb -> _OptionWakeup;
}

void EBYTE_SetFECMode(EBYTE * eb, uint8_t val) {
  eb -> _OptionFEC = val;
  EBYTE_BuildOptionByte(eb);
}

uint8_t EBYTE_GetFECMode(EBYTE * eb) {
  return eb -> _OptionFEC;
}

void EBYTE_SetTransmitPower(EBYTE * eb, uint8_t val) {
  eb -> _OptionPower = val;
  EBYTE_BuildOptionByte(eb);
}

uint8_t EBYTE_GetTransmitPower(EBYTE * eb) {
  return eb -> _OptionPower;
}

// Função para configurar o endereço
void EBYTE_SetAddress(EBYTE * eb, uint16_t val) {
  eb -> _AddressHigh = (val >> 8);
  eb -> _AddressLow = (val & 0xFF);
}

// Função para obter o endereço
uint16_t EBYTE_GetAddress(EBYTE * eb) {
  return (eb -> _AddressHigh << 8) | eb -> _AddressLow;
}

// Função para configurar a taxa de baud do UART
void EBYTE_SetUARTBaudRate(EBYTE * eb, uint8_t val) {
  eb -> _UARTDataRate = val;
  EBYTE_BuildSpeedByte(eb);
}

// Função para obter a taxa de baud do UART
uint8_t EBYTE_GetUARTBaudRate(EBYTE * eb) {
  return eb -> _UARTDataRate;
}

// Função para construir o byte de velocidade
void EBYTE_BuildSpeedByte(EBYTE * eb) {
  eb -> _Speed = 0;
  eb -> _Speed = ((eb -> _ParityBit & 0xFF) << 6) | ((eb -> _UARTDataRate & 0xFF) << 3) | (eb -> _AirDataRate & 0xFF);
}

// Função para construir o byte de opções
void EBYTE_BuildOptionByte(EBYTE * eb) {
  eb -> _Options = 0;
  eb -> _Options = ((eb -> _OptionTrans & 0xFF) << 7) | ((eb -> _OptionPullup & 0xFF) << 6) | ((eb -> _OptionWakeup & 0xFF) << 3) | ((eb -> _OptionFEC & 0xFF) << 2) | (eb -> _OptionPower & 0b11);
}

// Função para obter o estado do pino AUX
bool EBYTE_GetAux(EBYTE * eb) {
  return pinMSPRead(&P1IN, eb -> _AUX); // Supondo a existência de uma função pinMSPRead
}

void EBYTE_SaveParameters(EBYTE * eb, uint8_t val) {
  SetMode(eb, MODE_PROGRAM);

  UART_send_byte(val);
  UART_send_byte(eb -> _AddressHigh);
  UART_send_byte(eb -> _AddressLow);
  UART_send_byte(eb -> _Speed);
  UART_send_byte(eb -> _Channel);
  UART_send_byte(eb -> _Options);

  __delay_cycles(PIN_RECOVER);

  CompleteTask(eb, 4000);

  SetMode(eb, MODE_NORMAL);
}

// Função para imprimir os parâmetros
void EBYTE_PrintParameters(EBYTE * eb) {
  eb -> _ParityBit = (eb -> _Speed & 0XC0) >> 6;
  eb -> _UARTDataRate = (eb -> _Speed & 0X38) >> 3;
  eb -> _AirDataRate = eb -> _Speed & 0X07;

  eb -> _OptionTrans = (eb -> _Options & 0X80) >> 7;
  eb -> _OptionPullup = (eb -> _Options & 0X40) >> 6;
  eb -> _OptionWakeup = (eb -> _Options & 0X38) >> 3;
  eb -> _OptionFEC = (eb -> _Options & 0X07) >> 2;
  eb -> _OptionPower = (eb -> _Options & 0X03);
}

// Função ReadParameters
bool ReadParameters(EBYTE * eb) {
  memset(eb -> _Params, 0, sizeof(eb -> _Params)); // Zera os parâmetros

  SetMode(eb, MODE_PROGRAM);

  UART_send_byte(0xC1);
  UART_send_byte(0xC1);
  UART_send_byte(0xC1);

  for (int i = 0; i < sizeof(eb -> _Params); i++) {
    eb -> _Params[i] = UART_receive_byte();
  }

  eb -> _Save = eb -> _Params[0];
  eb -> _AddressHigh = eb -> _Params[1];
  eb -> _AddressLow = eb -> _Params[2];
  eb -> _Speed = eb -> _Params[3];
  eb -> _Channel = eb -> _Params[4];
  eb -> _Options = eb -> _Params[5];

  eb -> _Address = (eb -> _AddressHigh << 8) | (eb -> _AddressLow);
  eb -> _ParityBit = (eb -> _Speed & 0XC0) >> 6;
  eb -> _UARTDataRate = (eb -> _Speed & 0X38) >> 3;
  eb -> _AirDataRate = eb -> _Speed & 0X07;

  eb -> _OptionTrans = (eb -> _Options & 0X80) >> 7;
  eb -> _OptionPullup = (eb -> _Options & 0X40) >> 6;
  eb -> _OptionWakeup = (eb -> _Options & 0X38) >> 3;
  eb -> _OptionFEC = (eb -> _Options & 0X07) >> 2;
  eb -> _OptionPower = (eb -> _Options & 0X03);

  SetMode(eb, MODE_NORMAL);

  return (0xC0 == eb -> _Params[0]);
}

bool ReadModelData(EBYTE * eb) {
  memset(eb -> _Params, 0, sizeof(eb -> _Params)); // Zera os parâmetros

  bool found = false;
  int i = 0;

  SetMode(eb, MODE_PROGRAM); // Definir modo do módulo para Programação

  UART_send_byte(0xC3);
  UART_send_byte(0xC3);
  UART_send_byte(0xC3);

  for (i = 0; i < sizeof(eb -> _Params); i++) {
    eb -> _Params[i] = UART_receive_byte();
  }

  eb -> _Save = eb -> _Params[0];
  eb -> _Model = eb -> _Params[1];
  eb -> _Version = eb -> _Params[2];
  eb -> _Features = eb -> _Params[3];

  if (0xC3 != eb -> _Params[0]) {
    for (i = 0; i < 5; i++) {
      memset(eb -> _Params, 0, sizeof(eb -> _Params));

      UART_send_byte(0xC3);
      UART_send_byte(0xC3);
      UART_send_byte(0xC3);

      for (int j = 0; j < sizeof(eb -> _Params); j++) {
        eb -> _Params[j] = UART_receive_byte();
      }

      if (0xC3 == eb -> _Params[0]) {
        found = true;
        break;
      }

      __delay_cycles(10000);
    }
  } else {
    found = true;
  }

  SetMode(eb, MODE_NORMAL); // Retorna ao modo normal

  return found;
}

// Função para obter o modelo do módulo
uint8_t EBYTE_GetModel(EBYTE * eb) {
  return eb -> _Model;
}

// Função para obter a versão do módulo
uint8_t EBYTE_GetVersion(EBYTE * eb) {
  return eb -> _Version;
}

// Função para obter as características do módulo
uint8_t EBYTE_GetFeatures(EBYTE * eb) {
  return eb -> _Features;
}

void ClearBuffer() {
  unsigned long start_time = millis2();

  while (UART_available()) {
    UART_receive_byte(); // Lê e descarta o byte

    if ((millis2() - start_time) > 5000) {
      // Timeout após 5 segundos para evitar loop infinito
      //printf("Runaway buffer\n");
      break;
    }
  }
}
