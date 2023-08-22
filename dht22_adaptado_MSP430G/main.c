/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  -
 *
 *        Version:  1.0
 *        Created:  21/08/2023 10:17:24
 *       Revision:  none
 *       Compiler:  -
 *
 *         Author:  Isaac Vinicius, isaacvinicius2121@alu.ufc.br
 *   Organization:  UFC-Quixadá
 *
 * =====================================================================================
 */

#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Macros para os pinos GPIO para comunicacao com o DHT22 */
#define DHT22_PORT_OUT  P2OUT
#define DHT22_PORT_DIR  P2DIR
#define DHT22_PIN       BIT4 /* P2.4 */
#define DHT22_PORT_IN   P2IN

/* Vetores para capturar os dados do sensor */
volatile int temp[50]; /* 50 caso queira obter data e hora */
volatile int diff[50]; /* 50 caso queira obter data e hora */
volatile unsigned int i = 0;
volatile unsigned int j = 0;

/* Variaveis que irao receber os valores de umidade alta, umidade baixa, temperatura alta, temperatura baixa, bit de paridade */
volatile int int_humidity_high = 0;
volatile int int_humidity_low = 0;
volatile int int_temperature_high = 0;
volatile int int_temperature_low = 0;
volatile int check = 0;
volatile int checksum = 0;
volatile int dataok;

/* Strings para formatacao da saida de serial */
char temperatura[] = "Temperatura: ";
char ponto[] = ".";
char graus[] = "°C\n\r";
char umididade[] = "Umidade: ";
char porcento[] = "%\n\r";

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  uart_puts
 *  Description:  Funcao que faz o envio de dados para a serial
 * =====================================================================================
 */
void uart_puts(char * str) {
    while( * str != 0) {
        while(!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = * str++;
    }
}
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  int_to_string
 *  Description:  Funcao que converte valores inteiros em strings
 * =====================================================================================
 */
void int_to_string(int num, char * str) {
    int i = 0;
    int is_negative = 0;

    /* Lidando com numeros negativos */
    if(num < 0) {
        is_negative = 1;
        num = -num;
    }

    /* Convertendo numero em string, comecando pelo digito menos significativo */
    do {
        str[i++] = num % 10 + '0';  /* Adicionando digito a string */
        num /= 10;  /* Removendo digito do numero original */
    }
    while(num > 0);

    /* Invertendo a string para obter o valor correto */
    int j;
    for(j = 0; j < i / 2; j++) {
        char temp = str[j];  /* Armazenando temporariamente o caractere atual */
        str[j] = str[i - j - 1];  /* Substituindo o caractere atual pelo seu correspondente invertido */
        str[i - j - 1] = temp;  /* Colocando o caractere invertido na posição original */
    }

    /* Adicionando sinal negativo, se necessario */
    if(is_negative) {
        str[i++] = '-';  /* Adicionando o sinal negativo */
    }

    /* Adicionando terminador nulo */
    str[i] = '\0';  /* Marcando o fim da string */
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  processDataDHT22
 *  Description:  Funcao que recebe os dados referentes a:
 *                  umidade alta, umidade baixa, temperatura alta e temperatura baixa.
 *                  O processamento dos dados refere-se a conversao para umidade e
 *                  temperatura real.
 *
 *                  Datasheet:https://files.seeedstudio.com/wiki/Grove-Temperature_and_Humidity_Sensor_Pro/res/AM2302-EN.pdf
 *
 * =====================================================================================
 */
void processDataDHT22(unsigned char humidity_high, unsigned char humidity_low, unsigned char temperature_high, unsigned char temperature_low) {
    /* Calculando a umidade e a temperatura reais a partir dos dados lidos */
    unsigned int humidity_raw = (humidity_high << 8) | humidity_low;
    unsigned int temperature_raw = ((temperature_high & 0x7F) << 8) | temperature_low;

    /* Calculando a umidade e temperatura reais */
    float humidity_real = humidity_raw / 10.0f;
    float temperature_real = temperature_raw / 10.0f;

    if(temperature_high & 0x80) {
        /* Identificando temperatura negativa (bit mais significativo igual a 1) */
        temperature_real = -temperature_real;
    }

    /* Calculando a parte decimal dos valores de umidade e temperatura */
    int humidity_decimal = (int)((humidity_real - (int) humidity_real) * 10);
    int temperature_decimal = (int)((temperature_real - (int) temperature_real) * 10);

    /* Convertendo os valores inteiros para strings */
    char humidity_str[10];
    char temperature_str[10];
    int_to_string((int) humidity_real, humidity_str);
    int_to_string((int) temperature_real, temperature_str);

    /* Adicionando a parte decimal aos valores em string */
    char humidity_decimal_str[3];
    char temperature_decimal_str[3];

    /* Convertendo os valores inteiros para strings */
    int_to_string(humidity_decimal, humidity_decimal_str);
    int_to_string(temperature_decimal, temperature_decimal_str);

    /* Exibindo os valores de umidade e temperatura com uma casa decimal. Exemplo: umidade: 66.5% Temperatura: 30.1% */
    uart_puts(umididade);uart_puts(humidity_str); uart_puts(ponto); uart_puts(humidity_decimal_str); uart_puts(porcento);
    uart_puts(temperatura); uart_puts(temperature_str); uart_puts(ponto); uart_puts(temperature_decimal_str); uart_puts(graus);
}


int main(void) {
    /* Desativando o Watchdog Timer para evitar resets inesperados */
    WDTCTL = WDTPW | WDTHOLD;

    /* Configurando o sistema de clock para operar a 1MHz */
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    /* Configuracoes para comunicação serial (UART) a 19200 baud rate */
    /* Multiplexando os pinos P1.1 e P1.2 para UART */
    P1SEL = BIT1 | BIT2;
    P1SEL2 = BIT1 | BIT2;

    /* Configuracao do modulo de comunicacao UART */
    /* Habilitando o modo de configuracao da UART, colocando o modulo em modo de reset e selecionando SMCLK como fonte de clock */
    UCA0CTL1 |= UCSWRST + UCSSEL_2;

    /* Define os divisores de baud rate para atingir 19200 baud */
    UCA0BR0 = 52;
    UCA0BR1 = 0;

    /* Configurando o Modulador de Controle de Modulacao para ajuste fino do baud rate */
    UCA0MCTL = UCBRS_0;

    /* Desabilitando o modo de configuracao da UART */
    UCA0CTL1 &= ~UCSWRST;

  /* Delay de 2 segundos */
    __delay_cycles(2000000);

  /* Configurando o pino P2.4 como saida */
    DHT22_PORT_DIR |= DHT22_PIN;

  /* Configurando o pino P2.4 como nivel logico baixo */
    DHT22_PORT_OUT &= ~DHT22_PIN;

  /* Delay de 20ms */
    __delay_cycles(20000);

  /* Configurando o pino P2.4 como nivel logico ato */
    DHT22_PORT_OUT |= DHT22_PIN;

  /* Delay de 20us */
    __delay_cycles(20);

  /* Configurando o pino P2.4 como entrada */
    DHT22_PORT_DIR &= ~DHT22_PIN;

  /* Habilitando o timer para capturar os dados do sensor */
    P2SEL |= DHT22_PIN; /* MUltiplexacao para funcao de timer */
    TA1CTL = TASSEL_2 | MC_2; /* SMCLK | contagem de modo continuo*/

  /*
   Configura o canal 2 do Timer1_A para operacao de captura.
   Os bits CAP e CCIE ativam a captura e a interrupcao de captura/compare, respectivamente.
   O bit CCIS_0 seleciona a entrada CCIxA como fonte para captura.
   O bit CM_2 configura o modo de captura em borda de subida e descida.
   O bit SCS seleciona a o modo de capture sychronize.
  */
    TA1CCTL2 = CAP | CCIE | CCIS_0 | CM_2 | SCS;

  /* Habilitando as interrupcoes globais */
    __enable_interrupt();

/* TODO:
   Umidade:
     A resolucao da umidade e de 16 bits, com os bits mais significativos na frente. O valor de
     umidade fornecido pelo sensor e 10 vezes o valor real da umidade.
   Temperatura:
     A resolucao da temperatura e de 16 bits, com os bits mais significativos na frente. O valor
     de temperatura fornecido pelo sensor e 10 vezes o valor real da temperatura.
     O bit mais significativo da temperatura (Bit15) igual a 1 indica uma temperatura negativa, e
     o bit mais significativo da temperatura (Bit15) igual a 0 indica uma temperatura positiva.
     Os demais bits (Bit14 a Bit0) representam o valor numerico da temperatura.
   Bit de paridade:
     Parity bit = umidade alta + umidade baixa + temperatura alta + temperatura baixa.
*/

    while(1) {

        /* Verificando se os 40 bits foram recebidos */
        if(i >= 40) {
            /* Bits para Umidade Alta */
            for(j = 1; j <= 8; j++) {
                /* Se valor o valor for maior que 110 (110 uSeg), Bit = 1, se nao, Bit = 0*/
                if(diff[j] >= 110) {
                    int_humidity_high |= (0x01 << (8 - j));
                }
            }

            /* Bits para Umidade baixa */
            for(j = 9; j <= 16; j++) {
                /* Se valor o valor for maior que 110 (110 uSeg), Bit = 1, se nao, Bit = 0*/
                if(diff[j] >= 110) {
                    int_humidity_low |= (0x01 << (16 - j));
                }
            }

            /* Bits para Temperatura alta */
            for(j = 17; j <= 24; j++) {
                /* Se valor o valor for maior que 110 (110 uSeg), Bit = 1, se nao, Bit = 0*/
                if(diff[j] >= 110) {
                    int_temperature_high |= (0x01 << (24 - j));
                }
            }

            /* Bits para Temperatura baixa */
            for(j = 25; j <= 32; j++) {
                /* Se valor o valor for maior que 110 (110 uSeg), Bit = 1, se nao, Bit = 0*/
                if(diff[j] >= 110) {
                    int_temperature_low |= (0x01 << (32 - j));
                }
            }

            /* Bits para o Bit de paridade */
            for(j = 33; j <= 40; j++) {
            /* Se valor o valor for maior que 110 (110 uSeg), Bit = 1, se nao, Bit = 0*/
                if(diff[j] >= 110) {
                    checksum |= (0x01 << (40 - j));
                }
            }
            /* Realizando o calculo do Bit de Paridade */
            check = int_humidity_high + int_humidity_low + int_temperature_high + int_temperature_low;
            if(check == checksum) {
                dataok = 1;
            }else{
                dataok = 0;
            }

            /* Realizando o processamento dos dados obtidos pelo sensor e fazendo casting para unsigned char*/
            processDataDHT22((unsigned char) int_humidity_high, (unsigned char) int_humidity_low, (unsigned char) int_temperature_high, (unsigned char) int_temperature_low);
            __delay_cycles(1000000);
            WDTCTL = WDT_MRST_0_064;
        }
    }
}

/* Rotina de interrupcao do Timer A1 */
#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer_A1(void) {

  /* Armazenando o valor de captura atual em temp */
  temp[i] = TA1CCR2;

  /* Incrementando o índice i para o proximo valor de captura */
  i += 1;

  /* Limpando a flag de interrupcao de captura */
  TA1CCTL2 &= ~CCIFG;

  /* Verificando se foi coletado pelo menos 2 valores de captura */
  if(i >= 2) {
  /* Calculando a diferença entre os valores de captura e armazenando em diff */
  diff[i - 1] = temp[i - 1] - temp[i - 2];
  }
}

