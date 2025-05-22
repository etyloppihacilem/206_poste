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
#define DECODE_LENGTH ((MESSAGE_LENGTH - 2) / 3)

#define TAU_TICKS 6250UL       // 250 µs × 25 MHz
#define BURST_MAX_TICKS 1500UL // 60 µs × 25 MHz

uint8_t  CPT_PULSE     = 0;
uint32_t CURRENT_EDGE  = 0;
uint32_t PREVIOUS_EDGE = 0;

char    MESSAGE[MESSAGE_LENGTH] = "";
uint8_t CPT_CHAR                = 0;

char    DECODE[DECODE_LENGTH + 1] = "";
uint8_t message_ready             = 0;

void init_ir(void) {
    LPC_PINCON->PINSEL4 &= ~(0b11 << 20); // P2.10 -> GPIO
    LPC_PINCON->PINSEL4 |= (0b01 << 20);  // P2.10 -> EINT0

    LPC_SC->EXTMODE  |= (1 << 0); 
    LPC_SC->EXTPOLAR |= (1 << 0); 
    LPC_SC->EXTINT    = 1;

    LPC_SC->PCONP |= (1 << 1); 
    LPC_TIM0->TCR  = (1 << 1); // Réinitialiser TIMER0
    LPC_TIM0->TCR  = (1 << 0); // Démarrer TIMER0


    NVIC_EnableIRQ(EINT0_IRQn);
}

void EINT0_IRQHandler(void) {
    LPC_SC->EXTINT = 0x1;          // ACK
    CURRENT_EDGE   = LPC_TIM0->TC; // ticks

    uint32_t diff_ticks = CURRENT_EDGE - PREVIOUS_EDGE;

    if (diff_ticks < BURST_MAX_TICKS) { // burst "1"
        if (++CPT_PULSE == 8) {
            CPT_PULSE = 0;
            if (CPT_CHAR < MESSAGE_LENGTH - 1)
                MESSAGE[CPT_CHAR++] = '1';
        }
    } else { // espace "0"
        CPT_PULSE   = 0;
        uint8_t NB0 = diff_ticks / TAU_TICKS; // nombre de "0"
        for (uint8_t i = 0; i < NB0 && CPT_CHAR < MESSAGE_LENGTH - 1; i++)
            MESSAGE[CPT_CHAR++] = '0';
    }

    if (CPT_CHAR >= MESSAGE_LENGTH - 1) {
        MESSAGE[CPT_CHAR] = '\0';
        message_ready     = 1;
    }

    PREVIOUS_EDGE = CURRENT_EDGE;
}

//-------------------------------------------------------------------
void parsing_message(char *DECODE) {
    char temp[DECODE_LENGTH + 1] = "";

    if (MESSAGE[0] != '1' || MESSAGE[1] != '0')
        return;

    if (CPT_CHAR < 16)
        return;

    uint16_t index_m = 2;
    uint8_t  sum     = 0;

    for (int i = 0; i < 12; i += 4) {
        uint8_t quartet = 0;
        for (int j = 0; j < 4; j++)
            if (MESSAGE[index_m + i + j] == '1')
                quartet |= (1 << (3 - j));
        sum += quartet;
    }

    uint8_t checksum_expected = (~sum + 1) & 0xF;
    uint8_t checksum_received = 0;
    for (int i = 0; i < 4; i++)
        if (MESSAGE[index_m + 12 + i] == '1')
            checksum_received |= (1 << (3 - i));

    if (checksum_received != checksum_expected)
        return;

    // décodage « 100→0 », « 110→1 »
    uint16_t index_d = 0;
    while (MESSAGE[index_m] != '\0' && index_m + 2 < CPT_CHAR) {
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
