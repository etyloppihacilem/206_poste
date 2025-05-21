/* #####################################################################################################################

        _   ,_,   _         parsing_IR.c
       / `'=) (='` \        Created on 20 May. 2025 at 22h23
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/

#include "LPC17xx.h"

void init_display(void) {
    LPC_PINCON->PINSEL0 &= ~(0b1111111 << 8);
    LPC_PINCON->PINSEL1 &= ~(0b11111 << 2);
}

void disp_LED(char DECODE[]) {

    for (int i = 0; i < 12; i++)
        if (DECODE[i] == '1')
            LPC_GPIO1->FIOSET = (1 << i);
        else
            LPC_GPIO1->FIOCLR = (1 << i);
}