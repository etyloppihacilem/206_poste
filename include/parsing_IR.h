#pragma once

#include <stdint.h>

void init_ir(void);


typedef struct {
    uint8_t ID_rob;
    uint8_t vitesse;
    uint8_t status;
} message_IR;

message_IR *get_ir_msg();
void ir_msg_done();






