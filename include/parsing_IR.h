#pragma once

#include <stdint.h>

void init_ir(void);
void poste_msg_done();


typedef struct {
    uint8_t ID_rob;
    uint8_t vitesse;
    uint8_t status;
} message_IR;

message_IR *get_poste_msg();






