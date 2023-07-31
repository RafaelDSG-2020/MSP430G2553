// hx711.c

#include<msp430.h>

#define HX711_SCK_PORT  P1OUT
#define HX711_SCK_PIN   BIT4
#define HX711_DT_PORT   P1IN
#define HX711_DT_PIN    BIT3

void hx711_init()
{
     // Configuração dos pinos do HX711
    P1DIR |= HX711_SCK_PIN; // Define SCK como saída
    P1OUT &= ~HX711_SCK_PIN; // Garante que SCK esteja em LOW

    P1DIR &= ~HX711_DT_PIN; // Define DT como entrada
    P1REN |= HX711_DT_PIN; // Habilita resistor de pull-up
    P1OUT |= HX711_DT_PIN; // Define resistor de pull-up

    // Aguarde o HX711 estar pronto (estabilização)
    __delay_cycles(500); // Ajuste este valor conforme necessário (depende da estabilização do HX711)
}

long hx711_read()
 {
 //  / Inicializa a variável para armazenar o valor lido
    long value = 0;
    int i = 0;

    // Aguarda o HX711 estar pronto para a leitura
    while (P1IN & HX711_DT_PIN);

    // Faz a leitura dos 24 bits do HX711
    for ( i = 0; i < 24; i++) {
        P1OUT |= HX711_SCK_PIN; // SCK em HIGH
        __delay_cycles(50); // Pequeno atraso (ajuste conforme necessário)

        // Acumula o bit lido (os bits são lidos do MSB para o LSB)
        value = value << 1;
        if (P1IN & HX711_DT_PIN) {
            value++;
        }

        P1OUT &= ~HX711_SCK_PIN; // SCK em LOW
        __delay_cycles(60); // Pequeno atraso (ajuste conforme necessário)
    }

    // Faz a leitura do bit de sinal (25º bit)
    P1OUT |= HX711_SCK_PIN; // SCK em HIGH
    __delay_cycles(10); // Pequeno atraso (ajuste conforme necessário)
    value = value ^ 0x800000; // Inverte o bit de sinal (se necessário)
    P1OUT &= ~HX711_SCK_PIN; // SCK em LOW
   // __delay_cycles(10); // Pequeno atraso (ajuste conforme necessário)

    return value;
}
