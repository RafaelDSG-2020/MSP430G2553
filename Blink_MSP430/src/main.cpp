#include<msp430.h>


int main(void) 
{

    volatile unsigned int i;
    WDTCTL = WDTPW | WDTHOLD; // Parar o watchdog timer

    P1DIR |= 0x0001; // Configurar P1.0 como saída (LED)
    P1DIR &= ~(0x0008); // Configurar P1.3 como entrada (botão)
    P1REN |= (0x0008); // Habilitar resistor de pull-up interno no pino P1.3
   // P1OUT |= (0x0008); // Configurar resistor de pull-up como pull-up (nível alto)

    while (1) 
    {
        if ((P1IN & 0x0008) == 0)  // Verificar se o botão está pressionado (nível baixo)
        {
            P1OUT |= 0x0001; // Inverter o estado do LED (liga/desliga)
            for (i=10000; i>0; i--);
        }
        P1OUT &= ~(0x0001); // Inverter o estado do LED (liga/desliga)
    }

    return 0;
}

// //***********************************************
// //BLINK Definindo os valores dos registradores //
// //***********************************************

// #define LED1 (*((volatile unsigned char*)0x0022)) // Aponta para o endereço de memoria



// #define LIGA (*((volatile unsigned char*)0x0021)) // Aponta para o endereço de memoria



// //*********************************//
// //Funcionamento do acesso a memoria//
// // Endereço Valor                  //
// // 0x0022    0x01                  //
// //*********************************// 

// int main (void)
// {
//     volatile unsigned int i;
//     WDTCTL = WDTPW + WDTHOLD; // Desativando watdog;

//     LED1 |= 0x01; // Adiciono o valor no registrador;
    
//     while (1)
//     {
//       LIGA ^= 0x01;
//       for (i=10000; i>0; i--);
//     }
  
// }


//******************************************************
//BLINK Atraves das palavras reservadas da biblioteca //
//******************************************************
// int main (void)
// {
//     volatile unsigned int i;
//     WDTCTL = WDTPW + WDTHOLD; // Desativando watdog;

//     P1DIR |= 0x01; // Adiciono o valor no registrador;
    
//     while (1)
//     {
//       P1OUT ^= 0x01;
//       for (i=10000; i>0; i--);
//     }
// }
