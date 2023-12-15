#include <msp430.h>
#include <stdint.h>

#include "EBYTE.h"

// i recommend putting this code in a .h file and including it
// from both the receiver and sender modules
struct DATA
{
  unsigned long Count;
  int Bits;
  float Volts;
  float Amps;
};

// these are just dummy variables, replace with your own
int Chan;
DATA MyData;
unsigned long Last;

#define UART_RXD BIT1
#define UART_TXD BIT2
#define PIN_M0 BIT1
#define PIN_M1 BIT2
#define AUX_PIN BIT4 // Assumindo que o pino AUX está conectado ao P1.4

int UART_Init(void)
{
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

  return 1;
}


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Parar o watchdog timer

  EBYTE transmissor;
  EBYTE_Init(&transmissor, PIN_M0, PIN_M1, AUX_PIN);
  UART_Init();

  while(1) {
    if (UART_Init()) {
      GetStruct(&transmissor, &MyData, sizeof(MyData));
      Last = millis();
      
    }else{
      if ((millis() - Last) > 1000) {
          //Serial.println("Searching: ");
          Last = millis();
        }
    }
  }

  return 0;
}