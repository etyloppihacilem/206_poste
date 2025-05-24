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
    init_com_poste(115200);
    debug_write("coucou\r\n");
    init_ir();
    init_matrix();
    init_dtmf_peripheral();

    init_display();

    uint8_t dip_switch_value = DIP_switch();

    // char led_pattern[12] = "101010101010"; // Exemple : LEDs alternées ON/OFF

    // TODO: CHANGER LES PINS ET LA CLOCK POUR UART1 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    while (1) {

        if (msg_base_received) {
            msg_base_received = 0;

            // Messages IR
            message_IR *msg_ir;
            while ((msg_ir = get_ir_msg()) != 0) {
                send_msg_IR_to_base(msg_ir);
                ir_msg_done();
            }

            // Messages Pavé
            message_pave *msg_pave;
            while ((msg_pave = get_pave_msg()) != 0) {
                send_msg_pave_to_base(msg_pave);
                ir_pave_msg_done();
            }
            base_msg_done();
        }

        playDTMF(dip_switch_value);
    }


return 0;
}