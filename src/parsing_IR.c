#include "parsing_IR.h"
#include "LPC17xx.h"
#include "UART_base.h"
#include <stdint.h>

/* ---------------- DEBUG CONFIG ---------------- */
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


#define TAU_US 250UL //250 µs 


#define MESSAGE_LENGTH 50
#define LENGTH_FIFO_IR 16


static char    MESSAGE[MESSAGE_LENGTH] = { 0 };
static uint8_t MSG_IDX                 = 0;

static uint32_t lt_us = 0; 
static uint32_t tc_us = 0; 

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

    if ((uint8_t) (~(id + vel + st) + 1) != chk) {
        TRACE_STR("CHK ERR\r\n");
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
        TRACE_CHAR(b); /* “0” ou “1” en live */
    } else {
        MSG_IDX = 0;
    }
    parsing_message();
}

void init_ir(void) {
    LPC_PINCON->PINSEL4 &= ~(3 << 20);
    LPC_PINCON->PINSEL4 |= (1 << 20); // intéruption p2.10 en rising edge
    LPC_SC->EXTMODE     |= 1;
    LPC_SC->EXTPOLAR    |= 1;
    LPC_SC->EXTINT       = 1;

#if DEBUG_LED
    LED_PORT->FIODIR |= (1 << LED_PIN);
#endif

    /* Timer0 libre‑courant : 1 µs par tick ------------------------ */
    LPC_SC->PCONP |= (1 << 1);
    LPC_TIM0->PR   = 24; /* 24 pour 25 MHz CPU          */
    LPC_TIM0->TCR  = 2;  /* reset TC & PC               */
    LPC_TIM0->TCR  = 1;  /* start                       */

    NVIC_EnableIRQ(EINT0_IRQn);
}

void EINT0_IRQHandler(void) {
    LPC_SC->EXTINT = 1; // leve le flag

#if DEBUG_LED
    LED_PORT->FIOPIN ^= (1 << LED_PIN);
#endif

    uint32_t now_us = LPC_TIM0->TC; // temps actuel

    if ((lt_us - now_us) >= TAU_US) {

        uint8_t bit0 = (uint8_t) ((now_us - lt_us) / TAU_US);
        uint8_t bit1 = (uint8_t) (lt_us / TAU_US);


        while(bit1--)
        {
            push_bit('1');
        }

        while (bit0--) {
            push_bit('0');
        }

        LPC_TIM0->TCR = 2; // reset tc et lt
        LPC_TIM0->TCR = 1;
        lt_us         = 0;
    }
    lt_us           = now_us;
}

message_IR *get_ir_msg(void) {
    return (fifo_ir_r == fifo_ir_w) ? 0 : &fifo_ir[fifo_ir_r];
}

void ir_msg_done(void) {
    fifo_ir_r = (fifo_ir_r + 1) % LENGTH_FIFO_IR;
}
