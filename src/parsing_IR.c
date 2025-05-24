/* #####################################################################################################################

        _   ,_,   _         parsing_IR.c
       / `'=) (='` \        Created on 18 May. 2025 at 15:09
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/

#include "parsing_IR.h"
#include "LPC17xx.h"
#include "core_cmFunc.h"
#include <stdint.h>

#define MESSAGE_LENGTH 50
#define DECODE_LENGTH ((MESSAGE_LENGTH - 2) / 3)

#define TAU_TICKS 6250UL       // 250 µs × 25 MHz
#define BURST_MAX_TICKS 1500UL // 60 µs × 25 MHz

#define LENGTH_FIFO_IR 16

uint8_t  CPT_PULSE     = 0;
uint32_t CURRENT_EDGE  = 0;
uint32_t PREVIOUS_EDGE = 0;

char    MESSAGE[MESSAGE_LENGTH] = "";
uint8_t CPT_CHAR                = 0;

message_IR fifo_ir[LENGTH_FIFO_IR];
uint8_t    fifo_ir_r = 0;
uint8_t    fifo_ir_w = 0;

static void parsing_message() {
    uint16_t ir_decode = 0;

    if (MESSAGE[0] != '1' || MESSAGE[1] != '0')
        return;

    if (CPT_CHAR < 16)
        return;

    uint16_t index_m = 2;

    while (MESSAGE[index_m] != '\0' && index_m + 2 < CPT_CHAR) {
        if (MESSAGE[index_m] == '1' && MESSAGE[index_m + 1] == '0' && MESSAGE[index_m + 2] == '0') {
            ir_decode = ir_decode << 1;
            index_m  += 3;
        } else if (MESSAGE[index_m] == '1' && MESSAGE[index_m + 1] == '1' && MESSAGE[index_m + 2] == '0') {
            ir_decode = ir_decode << 1 | 1;
            index_m  += 3;
        } else {
            break;
        }
    }
    CPT_CHAR = 0;

    fifo_ir[fifo_ir_w].ID_rob  = (ir_decode & 0xf << 12) >> 12;
    fifo_ir[fifo_ir_w].vitesse = (ir_decode & 0xf << 8) >> 8;
    fifo_ir[fifo_ir_w].status  = (ir_decode & 0xf << 4) >> 4;

    if (((~(fifo_ir[fifo_ir_w].ID_rob + fifo_ir[fifo_ir_w].vitesse + fifo_ir[fifo_ir_w].status) + 1) & 0xf)
        == (ir_decode & 0xf)) {
        if (fifo_ir_w >= LENGTH_FIFO_IR - 1)
            fifo_ir_w = 0;
        else
            fifo_ir_w++;
    }
    return;
}

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

    if (CPT_CHAR >= MESSAGE_LENGTH - 1)
        MESSAGE[CPT_CHAR] = '\0';

    PREVIOUS_EDGE = CURRENT_EDGE;
    parsing_message();
}

/*
 * returns message to treat, or NULL
 * */
message_IR *get_ir_msg() {
    message_IR *msg_IR;

    if (fifo_ir_r == fifo_ir_w)
        msg_IR = 0;
    else
        msg_IR = fifo_ir + fifo_ir_r; // on retourne un pointeur sur le message

    return msg_IR;
}

/*
 * call this when done with processing message.
 * */
void ir_msg_done() {

    if (fifo_ir_r == LENGTH_FIFO_IR - 1)
        fifo_ir_r = 0;
    else
        fifo_ir_r++;
}