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
    printf("1\r\n");
    init_ir(); // 1
    printf("2\r\n");
    init_matrix(); // 2
    printf("3\r\n");
    init_dtmf_peripheral(); // 3
    printf("4\r\n");
    init_display(); // 4
    


    char decode_IR[16] = "";
    uint8_t dip_switch_value = DIP_switch();
    printf("coucou\r\n");


    while (1) {
        char *request = register_request();
        if(request != NULL){
            //ENVOI EN UART
        }

        playDTMF(dip_switch_value);

        if (message_ready) {
            parsing_message(decode_IR);
            message_ready = 0;

        }
        
        disp_LED(decode_IR);
        
    }

    return 0;
}