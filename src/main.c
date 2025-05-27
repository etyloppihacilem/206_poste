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

    /* 2. Matrice clavier + DIP ------------------------------------ */
    init_matrix();
    DIP_switch();
    // init_dtmf_peripheral(); // Initialisation du module DTMF

    /* 3.  >>>  IR  <<<  — indispensable ---------------------------- */
    init_ir(); // ← décommente cette ligne

    /* 5. Boucle principale ---------------------------------------- */
    // debug_put_uint(num_poste);
    while (1) {
        /* clavier */
        register_request();

        /* messages IR */
        message_IR *msg_ir;
        while ((msg_ir = get_ir_msg()) != 0) {
            send_msg_IR_to_base(msg_ir);
            debug_write("TX: ID=");
            debug_put_uint(msg_ir->ID_rob);
            debug_write(" V=");
            debug_put_uint(msg_ir->vitesse);
            debug_write(" S=");
            debug_put_uint(msg_ir->status);
            debug_write("\r\n");
            ir_msg_done();
        }

        // playDTMF(num_poste);

        /* messages pavé */
        if (msg_base_received) {
            message_pave *msg_pave = get_pave_msg();
            if (msg_pave != 0) {
                send_msg_pave_to_base(msg_pave);
                ir_pave_msg_done();
            } else {
                debug_write("NULL\r\n");
                msg_base_received = 0;
            }
        }
    }
}
