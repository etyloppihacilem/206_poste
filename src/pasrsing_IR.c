/* #####################################################################################################################

        _   ,_,   _         DTMF.c
       / `'=) (='` \        Created on 18 May. 2025 at 15:09
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/

#include "LPC17xx.h"
#include <stdint.h>

#define MESSAGE_LENGTH 51
#define TAU 250e-6
#define CLK_FREQ 25000000

uint8_t CPT_PULSE               = 0;
float   CURRENT_EDGE            = 0;
float   PREVIOUS_EDGE           = 0;
char    MESSAGE[MESSAGE_LENGTH] = "";
uint8_t CPT_CHAR                = 0;

void init_ir(void) {
    LPC_PINCON->PINSEL4 |= (0b1 << 20); // P2.10 -> EINT0

    LPC_SC->EXTMODE  |= 0b1; // EINT0 edge sensitive
    LPC_SC->EXTPOLAR |= 0b1; // rising_edge
    NVIC_EnableIRQ(EINT0_IRQn);

    LPC_TIM2->TCR = 0b10; // reset
    LPC_TIM2->TCR = 0b01; // enable tim2
}

void EINT0_IRQHandler(void) {
    CPT_PULSE++;
    CURRENT_EDGE = (LPC_TIM2->TC) / CLK_FREQ;

    if (CURRENT_EDGE - PREVIOUS_EDGE < 50e-6) // Inside burst
    {
        CPT_PULSE++;
        if (CPT_PULSE == 8) {
            CPT_PULSE           = 0;
            MESSAGE[CPT_CHAR++] = '1';
        }
    } else {
        uint8_t i   = 0;
        uint8_t NB0 = (int) (((CURRENT_EDGE - PREVIOUS_EDGE) / TAU) + 0.5); // nearest integer
        while (i++ < NB0)
            MESSAGE[CPT_CHAR++] = '0';
    }
    PREVIOUS_EDGE = CURRENT_EDGE;

    if (CPT_CHAR == 2 && (MESSAGE[0] != '1' || MESSAGE[1] != '0'))
        CPT_CHAR = 0;

    if (CPT_CHAR == MESSAGE_LENGTH)
        MESSAGE[CPT_CHAR] = '\0';
}
