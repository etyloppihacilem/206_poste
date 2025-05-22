#include "DTMF.h"
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
    init_com_super(9600);
    printf("Communication série initialisée.\r\n");

    init_ir();
    printf("Récepteur IR initialisé.\r\n");

    init_matrix();
    printf("Pavé matriciel initialisé.\r\n");

    init_dtmf_peripheral();
    printf("Périphérique DTMF initialisé.\r\n");

    init_display();
    printf("Affichage initialisé.\r\n");

    char decode_IR[16] = "";
    uint8_t dip_switch_value = DIP_switch();
    printf("Valeur des DIP switches : %d\r\n", dip_switch_value);

    char led_pattern[12] = "101010101010"; // Exemple : LEDs alternées ON/OFF

    while (1) {
        
        char *request = register_request();

        if (request != NULL) {
            printf("Requête détectée : %s\r\n", request);
            for (int i = 0; request[i] != '\0'; i++) {
                printf("Caractère %d : %c (ASCII : %d)\r\n", i, request[i], request[i]);
            }
        }

        if (dip_switch_value < 16) { 
            playDTMF(dip_switch_value);
        } else {
            printf("Valeur DIP switch invalide : %d\r\n", dip_switch_value);
        }

        if (message_ready) {
            parsing_message(decode_IR);
            message_ready = 0;

            if (decode_IR[0] != '\0') {
                //disp_LED(decode_IR);
            } else {
                printf("decode_IR est vide, rien à afficher sur les LEDs.\r\n");
            }
        }
        
        disp_LED(led_pattern);


    }

    return 0;
}