/* #####################################################################################################################

        _   ,_,   _         DTMF.c
       / `'=) (='` \        Created on 18 May. 2025 at 10:35
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

##################################################################################################################### */

#include "LPC17xx.h"
#include <stdint.h>

#define SAMPLE_RATE 44000U // 44 kHz
#define NUM_POINT_SIN 128

static const uint16_t sine_table_dac[NUM_POINT_SIN]
    = { 512, 531, 551, 570, 590, 609, 628, 646, 665, 683, 700, 717, 734, 750, 765, 780, 794, 808, 821, 833, 844, 855,
        864, 873, 881, 888, 894, 900, 904, 907, 910, 911, 912, 911, 910, 907, 904, 900, 894, 888, 881, 873, 864, 855,
        844, 833, 821, 808, 794, 780, 765, 750, 734, 717, 700, 683, 665, 646, 628, 609, 590, 570, 551, 531, 512, 492,
        472, 453, 433, 414, 395, 377, 358, 340, 323, 306, 289, 273, 258, 243, 229, 215, 202, 190, 179, 168, 159, 150,
        142, 135, 129, 123, 119, 116, 113, 112, 112, 112, 113, 116, 119, 123, 129, 135, 142, 150, 159, 168, 179, 190,
        202, 215, 229, 243, 258, 273, 289, 306, 323, 340, 358, 377, 395, 414, 433, 453, 472, 492 };

typedef struct {
        char  key;
        float f1;
        float f2;
} DTMF_t;

static const DTMF_t dtmf_table[16] = { { '1', 697, 1209 }, { '2', 697, 1336 }, { '3', 697, 1477 }, { 'A', 697, 1633 },
                                       { '4', 770, 1209 }, { '5', 770, 1336 }, { '6', 770, 1477 }, { 'B', 770, 1633 },
                                       { '7', 852, 1209 }, { '8', 852, 1336 }, { '9', 852, 1477 }, { 'C', 852, 1633 },
                                       { '*', 941, 1209 }, { '0', 941, 1336 }, { '#', 941, 1477 }, { 'D', 941, 1633 } };

uint32_t CPT_TIME = 0;    // compteur global d’échantillons
uint32_t CPT_200MS = 0;
uint32_t INCR_SEQ = 0;																				 
																			 
uint16_t F1 = 1.0f; 
uint16_t F2 = 1.0f;

static char     sequence[3] = { 'A', '0', '0' };


void init_dtmf_peripheral(void) {
    LPC_PINCON->PINSEL1 |= (2 << 20); // P0.26  AOUT

    LPC_SC->PCONP |= (1 << 22); // Power TIMER2
    LPC_TIM2->PR   = 0;
    LPC_TIM2->MR0  = 25000000U / SAMPLE_RATE;
    LPC_TIM2->MCR  = 0b11; // reset + IT sur MR0
    LPC_TIM2->TCR  = 2;
    LPC_TIM2->TCR  = 1; // reset puis enable
    NVIC_EnableIRQ(TIMER2_IRQn);
}

static void set_tone(char key) {
    for (uint8_t i = 0; i < 16; i++) {
        if (dtmf_table[i].key == key) {
            F1 = dtmf_table[i].f1;
            F2 = dtmf_table[i].f2;
            return;
        }
    }
}

void TIMER2_IRQHandler(void) {
    LPC_TIM2->IR = 1; // Clear interrupt flag

    float    t1 = ((float) (((CPT_TIME * F1) % SAMPLE_RATE) * NUM_POINT_SIN)) / ((float) SAMPLE_RATE);
    uint16_t V1 = sine_table_dac[(((uint16_t) t1) & 0xFF)];

    float    t2 = ((float) (((CPT_TIME * F2) % SAMPLE_RATE) * NUM_POINT_SIN)) / ((float) SAMPLE_RATE);
    uint16_t V2 = sine_table_dac[(((uint16_t) t2) & 0xFF)];

    uint32_t Vsum = ((V1 + V2) >> 1) & 0x3FF;
    LPC_DAC->DACR = Vsum << 6;
    if (++CPT_TIME >= SAMPLE_RATE)
        CPT_TIME = 0;
		
		CPT_200MS++;
		if (CPT_200MS >= 8800)
		{
			if(INCR_SEQ >= 3) INCR_SEQ = 0;
			set_tone(sequence[INCR_SEQ++]);
			CPT_200MS = 0;
		}
}




void playDTMF(uint8_t idPoste) {
    if (idPoste > 99)
        idPoste = 99;

    sequence[0] = 'A';
    sequence[1] = (idPoste / 10) + '0';
    sequence[2] = (idPoste % 10) + '0';


}

