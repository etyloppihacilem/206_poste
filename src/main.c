#include "parsing_IR.h"
#include "pave_DIP.h"
#include "stdint.h"
#include "com_super.h"
#include "sys/_intsup.h"
#include <LPC17xx.h>
#include <stdio.h>

int main(void) {
    init_com_super(9600);
    printf("1\r\n");
    init_ir();
    printf("2\r\n");
    init_matrix();
    printf("coucou\r\n");
    char dip_switch[4] = "";

    uint8_t col_value = matrix(4); // Appeler matrix avec l'index 4 pour la 5ème ligne
    for (int i = 0; i < 4; i++) {
        dip_switch[i] = (col_value & (1 << i)) ? '1' : '0'; // Convertir les bits en '1' ou '0'
    }

    while (1) {
        char* request = register_request();

        if (message_ready) {
            // parsing_message();
            message_ready = 0;
        }



        if (request) { 
            printf("Requête enregistrée : %s\r\n", request);
        }
    }

    return 0;
}