/* #####################################################################################################################

               """          com_super.c
        -\-    _|__
         |\___/  . \        Created on 16 May. 2025 at 14:35
         \     /(((/        by hmelica
          \___/)))/         hmelica@student.42.fr

#####################################################################################################################
*/

#include "com_super.h"
#include "LPC17xx.h"
#include <stdint.h>
#include <stdio.h>

#define MSG_LENGTH 32
#define INBOX_SIZE 4

char             msg_super[MSG_LENGTH + 1] = { 0 };

uint8_t c_super = 0;


void UART0_IRQHandler(void) {
    if (LPC_UART0->IIR & 0x04) // RDA: Receive Data Available
    {
        if (c_super == MSG_LENGTH)
            c_super = 0;
        char c               = LPC_UART0->RBR & 0xFF;
        msg_super[c_super++] = c;
        msg_super[c_super]   = '\0'; // to end string
    }
    if (c_super >= 2 && msg_super[c_super - 1] == '\n' && msg_super[c_super - 2] == '\r') {
        c_super            = 0;
        msg_super[c_super] = '\0'; // to end string
    }
}

void init_com_super(uint32_t baudrate) {
    LPC_SC->PCONP |= (1 << 3); // on active l'horloge et le power de UART 0

    LPC_PINCON->PINSEL0 &= ~((3 << 4) | (3 << 6));
    LPC_PINCON->PINSEL0 |= (1 << 4) | (1 << 6); // P0.2 = TXD0, P0.3 = RXD0

    uint32_t Fdiv;
    uint32_t pclk;

    // Le PCLK de UART0 est par défaut à CCLK / 4
    pclk = SystemCoreClock / 4;

    // Désactiver interruptions pendant init
    LPC_UART0->IER = 0;

    // 8 bits, 1 stop bit, pas de parité
    LPC_UART0->LCR = 0x83; // DLAB = 1 pour accéder à DLL/DLM

    Fdiv           = pclk / (16 * baudrate);
    LPC_UART0->DLM = Fdiv / 256;
    LPC_UART0->DLL = Fdiv % 256;

    LPC_UART0->LCR = 0x03; // DLAB = 0
    LPC_UART0->FCR = 0x07; // Activer FIFO, reset RX/TX

    // Attente que tout soit prêt
    while (!(LPC_UART0->LSR & 0x20))
        ;
    NVIC_EnableIRQ(UART0_IRQn);
    LPC_UART0->IER = 1; // RBR interrupt enable
}

// overload pour printf

static int uart0_putchar(int c) { // peut être bufferiser ça si les prints prennent trop de temps.
    while (!(LPC_UART0->LSR & 0x20))
        ; // attente THRE
    LPC_UART0->THR = c;
    return c;
}

int fputc(int c, FILE *f) { // not needed for gcc compiler
    (void) f;               // unused
    return uart0_putchar(c);
}

int _write(int file, char *ptr, int len) {
    (void) file; // unused
    for (int i = 0; i < len; i++)
        uart0_putchar(ptr[i]);
    return len;
}
