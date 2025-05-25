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
    // init_ir();
    init_matrix();
    // init_dtmf_peripheral();
    // init_display();
    DIP_switch();
    // char led_pattern[12] = "101010101010"; // Exemple : LEDs alternées ON/OFF

    // TODO: CHANGER LES PINS ET LA CLOCK POUR UART1 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    while (1) {
        /* → ajoute cette ligne */
        register_request();      //<– scanne le pavé à chaque tour

        /* traitement des messages vers la base */
        message_pave *msg_pave;
        while ((msg_pave = get_pave_msg()) != 0) {
            send_msg_pave_to_base(msg_pave);
            ir_pave_msg_done();
        }
        
    }


return 0;
}