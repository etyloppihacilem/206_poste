/* #####################################################################################################################

        _   ,_,   _         parsing_IR.c
       / `'=) (='` \        Created on 20 May. 2025 at 22h23
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/

#include "LPC17xx.h"

void init_display(void) {
    //Configurer P0.4 à P0.10 en GPIO
    LPC_PINCON->PINSEL0 &= ~(0b111111111111 << 8);
    LPC_GPIO0->FIODIR |= (0b1111111 << 4);        

    //Configurer P0.17 à P0.21 en GPIO
    LPC_PINCON->PINSEL1 &= ~(0b1111111111 << 2);  
    LPC_GPIO0->FIODIR |= (0b11111 << 17);         
}

void disp_LED(char *pattern) {
    
    LPC_GPIO0->FIOCLR = (0b1111111 << 4) | (0b11111 << 17);

    // Appliquer le nouveau motif
    for (int i = 0; i < 12; i++) {
        if (pattern[i] == '1') {
            if (i < 7) {
                LPC_GPIO0->FIOSET = (1 << (4 + i)); // LEDs P0.4 à P0.10
            } else {
                LPC_GPIO0->FIOSET = (1 << (17 + (i - 7))); // LEDs P0.17 à P0.21
            }
        }
    }
}