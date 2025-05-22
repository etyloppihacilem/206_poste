#include "DTMF.h"
#include "com_super.h"
#include "display.h"
#include "parsing_IR.h"
#include "pave_DIP.h"
#include "stdint.h"
#include "sys/_intsup.h"
#include <LPC17xx.h>
#include <stdio.h>

int main(void) {
    init_com_super(9600);
    printf("1\r\n");
    init_ir(); // 1
    printf("2\r\n");
    init_matrix(); // 2
    printf("3\r\n");
    init_dtmf_peripheral(); // 3
    printf("4\r\n");
    init_display(); // 4
    printf("coucou\r\n");

    char dip_switch[4] = "";
    char decode_IR[16] = "";

    uint8_t col_value = matrix(4); // Appeler matrix avec l'index 4 pour la 5ème ligne
    for (int i = 0; i < 4; i++)
        dip_switch[i] = (col_value & (1 << i)) ? '1' : '0'; // Convertir les bits en '1' ou '0'

    while (1) {
        char *request = register_request();

        if (message_ready) {
            parsing_message(decode_IR);
            message_ready = 0;
        }

        if (request)
            printf("Requête enregistrée : %s\r\n", request);
    }

    return 0;
}