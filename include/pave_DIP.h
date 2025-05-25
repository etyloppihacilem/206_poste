#pragma once
#include "parsing_IR.h"
#include <stdint.h>

void    init_matrix(void);
uint8_t parsing_pave(uint8_t y);
uint8_t matrix(uint8_t y);

void DIP_switch();

typedef struct {
    char ID_post_uni;
    char ID_post_dix;
    char livreur;
} message_pave;

message_pave *get_pave_msg();
void          ir_pave_msg_done();
void register_request(void);

extern uint8_t num_poste;
