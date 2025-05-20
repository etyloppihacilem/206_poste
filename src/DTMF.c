/* #####################################################################################################################

        _   ,_,   _         DTMF.c
       / `'=) (='` \        Created on 18 May. 2025 at 10:35
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/

#include "LPC17xx.h"
#include <stdint.h>

#define SAMPLE_RATE 8000 // 8 kHz
#define NUM_POINT_SIN 127

uint16_t sine_table_dac[] = {
    512, 531, 551, 570, 590, 609, 628, 646, 665, 683, 700, 717, 734, 750, 765, 780, 794, 808, 821, 833, 844, 855,
    864, 873, 881, 888, 894, 900, 904, 907, 910, 911, 912, 911, 910, 907, 904, 900, 894, 888, 881, 873, 864, 855,
    844, 833, 821, 808, 794, 780, 765, 750, 734, 717, 700, 683, 665, 646, 628, 609, 590, 570, 551, 531, 512, 492,
    472, 453, 433, 414, 395, 377, 358, 340, 323, 306, 289, 273, 258, 243, 229, 215, 202, 190, 179, 168, 159, 150,
    142, 135, 129, 123, 119, 116, 113, 112, 112, 112, 113, 116, 119, 123, 129, 135, 142, 150, 159, 168, 179, 190,
    202, 215, 229, 243, 258, 273, 289, 306, 323, 340, 358, 377, 395, 414, 433, 453, 472, 492,
};

uint16_t CPT_TIME = 0;
float    CURRENT_TIME;
float    PER1 = 0;
float    PER2 = 0;

typedef struct {
        char  key;
        float freq1;
        float freq2;
} DTMF_t;

DTMF_t dtmf_table[] = { { '1', 697, 1209 }, { '2', 697, 1336 }, { '3', 697, 1477 }, { 'A', 697, 1633 },
                        { '4', 770, 1209 }, { '5', 770, 1336 }, { '6', 770, 1477 }, { 'B', 770, 1633 },
                        { '7', 852, 1209 }, { '8', 852, 1336 }, { '9', 852, 1477 }, { 'C', 852, 1633 },
                        { '*', 941, 1209 }, { '0', 941, 1336 }, { '#', 941, 1477 }, { 'D', 941, 1633 } };

void init_dtmf_peripheral(void) {
    // Initialize the DTMF peripheral configure TIMER 2 and ADC
    LPC_PINCON->PINSEL1 |= (0b1 << 21); // P0.26 = AOUT

    LPC_TIM2->TCR = 0b10;               // reset
    LPC_TIM2->MCR = 0b11;               // reset on MR0, interrupt on MR0
    LPC_TIM2->MR0 = 25e6 / SAMPLE_RATE; // Set the match register for the desired sample rate
    LPC_TIM2->TCR = 0b01;               // enable tim2
    NVIC_EnableIRQ(TIMER2_IRQn);
}

void TIMER2_IRQHandler(void) {
    LPC_TIM2->IR = 1; // Clear the interrupt flag
    CPT_TIME++;       // reset in play_DTMF

    CURRENT_TIME = (float) CPT_TIME / SAMPLE_RATE;
    float t1     = CURRENT_TIME;

    while (t1 > PER1) // back to first period
        t1 -= PER1;
    t1 /= PER1;

    uint16_t k1 = (uint16_t) (t1 * NUM_POINT_SIN);
    float    V1 = sine_table_dac[k1];

    float t2 = CURRENT_TIME;
    while (t2 > PER2) // back to first period
        t2 -= PER2;
    t2 /= PER2;

    uint16_t k2 = (uint16_t) (t2 * NUM_POINT_SIN);
    float    V2 = sine_table_dac[k2];

    uint32_t Vsum = ((uint32_t) V1 + (uint32_t) V2) / 2;

    LPC_DAC->DACR = Vsum << 6;
}

void play_dtmf(char key) {
    for (int i = 0; i < 16; i++) {
        if (dtmf_table[i].key == key) {
            PER1     = 1.0f / dtmf_table[i].freq1;
            PER2     = 1.0f / dtmf_table[i].freq2;
            CPT_TIME = 0;
            return;
        }
    }
}
