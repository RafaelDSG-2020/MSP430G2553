#include "EBYTE.h"

#include <stdlib.h>

#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1

extern unsigned long millis() {
  for (int i = 0; i < 5000; i++) {
    i++;
  }
}

void delayEBYTE(unsigned int value) {
  register unsigned int loops = ((F_CPU * value) >> 2);

  while (loops) {
    __delay_cycles(1);
    loops--;
  };
}

void pinMSPMode(volatile unsigned char & portDir, uint8_t pin, uint8_t mode) {
  if (mode == INPUT) {
    portDir &= ~(1 << pin); // Configura o pino como entrada
  } else if (mode == OUTPUT) {
    portDir |= (1 << pin); // Configura o pino como saída
  }
}

void pinMSPWrite(volatile unsigned char & portOut, uint8_t pin, uint8_t level) {
  if (level == LOW) {
    portOut &= ~(1 << pin); // Nível baixo
  } else if (level == HIGH) {
    portOut |= (1 << pin); // Nível alto
  }
}

int pinMSPRead(volatile unsigned char & port, uint8_t pin) {
  return (port & (1 << pin)) ? HIGH : LOW;
}
