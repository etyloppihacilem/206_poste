#include "parsing_IR.h"
#include "LPC17xx.h"
#include "UART_base.h"
#include <stdint.h>

#define DEBUG_IR 1
#define DEBUG_LED 1
#define LED_PORT LPC_GPIO0
#define LED_PIN 22

#if DEBUG_IR
# define TRACE_CHAR(c) UART0_putchar(c)
# define TRACE_STR(s) debug_write(s)
# define TRACE_UINT(u) debug_put_uint(u)
#else
# define TRACE_CHAR(c) ((void) 0)
# define TRACE_STR(s) ((void) 0)
# define TRACE_UINT(u) ((void) 0)
#endif

#define TAU_US 250UL
#define MESSAGE_LENGTH 54
#define LENGTH_FIFO_IR 16

static char    MESSAGE[MESSAGE_LENGTH] = { 0 };
static uint8_t MSG_IDX                 = 0;

static uint32_t last_edge_us    = 0; // dernier front montant
static uint8_t  first_edge      = 1;
static uint8_t  prev_level_high = 0;

static message_IR fifo_ir[LENGTH_FIFO_IR];
static uint8_t    fifo_ir_r = 0, fifo_ir_w = 0;

static void parsing_message(void) {
    if (MSG_IDX < 16 || MESSAGE[0] != '1' || MESSAGE[1] != '0')
        return;

    uint16_t ir = 0;
    for (uint8_t i = 2; i + 2 < MSG_IDX; i += 3)
        if (MESSAGE[i] == '1' && MESSAGE[i + 1] == '0' && MESSAGE[i + 2] == '0')
            ir <<= 1;
        else if (MESSAGE[i] == '1' && MESSAGE[i + 1] == '1' && MESSAGE[i + 2] == '0')
            ir = (ir << 1) | 1;
        else
            break;

    uint8_t id  = (ir >> 12) & 0xF;
    uint8_t vel = (ir >> 8) & 0xF;
    uint8_t st  = (ir >> 4) & 0xF;
    uint8_t chk = ir & 0xF;

    if ((uint8_t) ((~(id + vel + st) + 1) & 0xf) != chk) {
        TRACE_STR("ERREUR CHECK SUM\r\n");
        MSG_IDX    = 0;
        MESSAGE[0] = '\0';
        return;
    }

    fifo_ir[fifo_ir_w].ID_rob  = id;
    fifo_ir[fifo_ir_w].vitesse = vel;
    fifo_ir[fifo_ir_w].status  = st;
    fifo_ir_w                  = (fifo_ir_w + 1) % LENGTH_FIFO_IR;

    TRACE_STR("\r\n→ IR FIFO  ID=");
    TRACE_UINT(id);
    TRACE_STR(" V=");
    TRACE_UINT(vel);
    TRACE_STR(" S=");
    TRACE_UINT(st);
    TRACE_STR("\r\n");

    MSG_IDX    = 0;
    MESSAGE[0] = '\0';
}

static inline void push_bit(char b) {
    if (MSG_IDX < MESSAGE_LENGTH - 1) {
        MESSAGE[MSG_IDX++] = b;
        MESSAGE[MSG_IDX]   = '\0';
    } else {
        MSG_IDX = 0;
    }
    parsing_message();
}

void init_ir(void) {

    LPC_PINCON->PINSEL4    &= ~(3 << 24); // P2.12
    LPC_PINCON->PINMODE4   |= (3 << 24);
    LPC_GPIOINT->IO2IntEnR |= (1 << 12);
    LPC_GPIOINT->IO2IntEnF |= (1 << 12);

    LED_PORT->FIODIR |= 1 << LED_PIN;

    LPC_SC->PCONP |= (1 << 1);
    LPC_TIM0->PR   = 24;
    LPC_TIM0->TCR  = 2; // reset
    LPC_TIM0->TCR  = 1; // start

    NVIC_EnableIRQ(EINT3_IRQn);
}

void EINT3_IRQHandler(void) {
    if ((LPC_GPIOINT->IO2IntStatR | LPC_GPIOINT->IO2IntStatF) & (1 << 12)) { // dettection d'un front
        uint32_t now_us = LPC_TIM0->TC;

#if DEBUG_LED
        LED_PORT->FIOPIN ^= (1 << LED_PIN);
#endif

        if (!first_edge) {
            uint32_t duration = now_us - last_edge_us;
            uint8_t  nb_bits  = (uint8_t) ((duration + TAU_US / 2) / TAU_US); // nb bit pendant tau arondi
            char     bit_char = prev_level_high ? '1' : '0';

            for (uint8_t i = 0; i < nb_bits; ++i)
                push_bit(bit_char);
        } else {
            first_edge = 0; // preimier front
        }

        last_edge_us    = now_us;
        prev_level_high = (LPC_GPIO2->FIOPIN >> 10) & 1;
    }
    LPC_GPIOINT->IO2IntClr = (1 << 12); // leve les flag
}

message_IR *get_ir_msg(void) {
    return (fifo_ir_r == fifo_ir_w) ? 0 : &fifo_ir[fifo_ir_r];
}

void ir_msg_done(void) {
    fifo_ir_r = (fifo_ir_r + 1) % LENGTH_FIFO_IR;
}
