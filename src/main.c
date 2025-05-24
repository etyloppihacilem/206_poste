#include "DTMF.h"
#include "UART_base.h"
#include "com_super.h"
#include "display.h"
#include "parsing_IR.h"
#include "pave_DIP.h"
#include "stdint.h"
#include "sys/_intsup.h"
#include <LPC17xx.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    init_com_poste(115200);

    printf("Communication série initialisée.\r\n");

    init_ir();
    printf("Récepteur IR initialisé.\r\n");

    init_matrix();
    printf("Pavé matriciel initialisé.\r\n");

    init_dtmf_peripheral();
    printf("Périphérique DTMF initialisé.\r\n");

    init_display();

    uint8_t dip_switch_value = DIP_switch();

    // char led_pattern[12] = "101010101010"; // Exemple : LEDs alternées ON/OFF

    while (1) {

        if (msg_base_received) {
            msg_base_received = 0;

            // Messages IR
            message_IR *msg_ir;
            while ((msg_ir = get_ir_msg()) != NULL) {
                send_msg_IR_to_base(msg_ir);
                ir_msg_done();
            }

            // Messages Pavé
            message_pave *msg_pave;
            while ((msg_pave = get_pave_msg()) != NULL) {
                send_msg_pave_to_base(msg_pave);
                ir_pave_msg_done();
            }
            base_msg_done();
        }

        playDTMF(dip_switch_value);
    }


return 0;
}