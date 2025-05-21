/* #####################################################################################################################

        _   ,_,   _         parsing_IR.c
       / `'=) (='` \        Created on 18 May. 2025 at 15:09
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/

#include "LPC17xx.h"
#include <stdint.h>

#define MESSAGE_LENGTH 50
#define DECODE_LENGTH (MESSAGE_LENGTH - 2) / 3
#define TAU 250e-6
#define CLK_FREQ 25000000

uint8_t CPT_PULSE     = 0;
float   CURRENT_EDGE  = 0;
float   PREVIOUS_EDGE = 0;

char    MESSAGE[MESSAGE_LENGTH] = "";
uint8_t CPT_CHAR                = 0;

char    DECODE[DECODE_LENGTH + 1] = "";
uint8_t message_ready             = 0;

void init_ir(void) {

    LPC_PINCON->PINSEL4 &= ~(0b11 << 20); // Clear P2.10 bits
    LPC_PINCON->PINSEL4 |= (0b01 << 20);  // Set P2.10 to EINT0

    LPC_SC->EXTMODE  |= (1 << 0);  // EINT0 : edge sensitive
    LPC_SC->EXTPOLAR |= (1 << 0);  // pour front montant
    NVIC_EnableIRQ(EINT0_IRQn);

    LPC_SC->PCONP |= (1 << 22);
    LPC_TIM2->TCR  = (1 << 1); // Reset Timer
    LPC_TIM2->TCR  = (1 << 0); // Enable Timer
}

void EINT0_IRQHandler(void) {
    CURRENT_EDGE = (LPC_TIM2->TC) / CLK_FREQ;

    if (CURRENT_EDGE - PREVIOUS_EDGE < 60e-6) // Inside burst (1s)
    {
        CPT_PULSE++;
        if (CPT_PULSE == 8) {
            CPT_PULSE = 0;
            if (CPT_CHAR < MESSAGE_LENGTH - 1)
                MESSAGE[CPT_CHAR++] = '1';
        }
    } else // 0s
    {
        CPT_PULSE   = 0;
        uint8_t NB0 = (int) (((CURRENT_EDGE - PREVIOUS_EDGE) / TAU) + 0.5);
        for (uint8_t i = 0; i < NB0 && CPT_CHAR < MESSAGE_LENGTH - 1; i++)
            MESSAGE[CPT_CHAR++] = '0';
    }

    if (CPT_CHAR >= MESSAGE_LENGTH - 1) {
        MESSAGE[CPT_CHAR] = '\0';
        message_ready     = 1;
    }

    PREVIOUS_EDGE = CURRENT_EDGE;

    LPC_SC->EXTINT |= 0x1;
}



void parsing_message(char MESSAGE[], char DECODE[]) {
    char temp[DECODE_LENGTH + 1] = ""; 

    
    if (MESSAGE[0] != '1' || MESSAGE[1] != '0')  // Vérification du message de début
        return;

    uint16_t index_m = 2;
    uint16_t index_d = 0; 

    if (CPT_CHAR < 16) 
        return;

    // Calcul de la somme des 3 quartets précédents
    uint8_t sum = 0;
    for (int i = 0; i < 12; i += 4) {
        uint8_t quartet = 0;
        for (int j = 0; j < 4; j++) {
            if (MESSAGE[index_m + i + j] == '1') {
                quartet |= (1 << (3 - j)); // Construire le quartet en binaire
            }
        }
        sum += quartet; 
    }

    // complement a 2
    uint8_t checksum_expected = (~sum + 1) & 0xF; //limite le resultat a 4bit

    // recupere le checksum du message
    uint8_t checksum_received = 0;
    for (int i = 0; i < 4; i++) {
        if (MESSAGE[index_m + 12 + i] == '1') {
            checksum_received |= (1 << (3 - i)); 
        }
    }

    if (checksum_received != checksum_expected) {
        return; 
    }

    // parsing en 1 et 0
    while (index_m < 14 && MESSAGE[index_m] != '\0') {
        if (MESSAGE[index_m] == '1' && MESSAGE[index_m + 1] == '0' && MESSAGE[index_m + 2] == '0') {
            temp[index_d++] = '0';
            index_m        += 3;
        } else if (MESSAGE[index_m] == '1' && MESSAGE[index_m + 1] == '1' && MESSAGE[index_m + 2] == '0') {
            temp[index_d++] = '1';
            index_m        += 3;
        } else {
            break;
        }
    }

    temp[index_d] = '\0';
    for (int i = 0; i < DECODE_LENGTH + 1; i++)
        DECODE[i] = temp[i];

    CPT_CHAR = 0;
}