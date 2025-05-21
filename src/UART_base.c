/* #####################################################################################################################

        _   ,_,   _         parsing_IR.c
       / `'=) (='` \        Created on 21 May. 2025 at 20h52
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/

#include "LPC17xx.h"
#include <stdint.h>

void init_UART(void) {
    LPC_SC->PCONP |= (1 << 3); // active l'alim de l'UART

    LPC_PINCON->PINSEL0 &= ~((3 << 4) | (3 << 6));
    LPC_PINCON->PINSEL0 |= (1 << 4) | (1 << 6);

    //calcul de la freq de UART
    uint32_t Fdiv;
    uint32_t pclk;
    pclk = SystemCoreClock / 4;
    
    LPC_UART0->IER = 0; //desactivation de l'uart pendant la config 
    

}