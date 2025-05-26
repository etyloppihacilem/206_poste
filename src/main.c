#include "DTMF.h"
#include "UART_base.h"
#include "display.h"
#include "parsing_IR.h"
#include "pave_DIP.h"
#include "stdint.h"
#include "sys/_intsup.h"
#include <LPC17xx.h>
#include <stdint.h>

int main(void) {
    /* 1. UART debug ------------------------------------------------ */
    init_com_poste(115200);
    debug_write("coucou\r\n");

    /* 2. Matrice clavier + DIP ------------------------------------ */
    init_matrix();
    DIP_switch();

    /* 3.  >>>  IR  <<<  — indispensable ---------------------------- */
    init_ir(); // ← décommente cette ligne

    /* 4. (optionnel) activer globalement les IRQ si elles sont off  */
    __enable_irq(); // CMSIS ; la plupart des boot-code l’ont déjà fait

    /* 5. Boucle principale ---------------------------------------- */
    while (1) {
        /* clavier */
        register_request();

        /* messages IR */
        message_IR *msg_ir;
        while ((msg_ir = get_ir_msg()) != 0) {
            send_msg_IR_to_base(msg_ir);
            debug_write("TX: ID=");  debug_put_uint(msg_ir->ID_rob);
            debug_write(" V=");      debug_put_uint(msg_ir->vitesse);
            debug_write(" S=");      debug_put_uint(msg_ir->status);
            debug_write("\r\n");
            ir_msg_done();
        }

        /* messages pavé */
        message_pave *msg_pave;
        while ((msg_pave = get_pave_msg()) != 0) {
            send_msg_pave_to_base(msg_pave);
            ir_pave_msg_done();
        }
    }
}
