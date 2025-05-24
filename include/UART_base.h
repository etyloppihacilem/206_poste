#pragma once

#include <stdint.h>
#include "parsing_IR.h"
#include "pave_DIP.h"

typedef enum {
    p_vide = 0,
    EOT    = 'N',
} t_msg_poste_type;

typedef struct {
        t_msg_poste_type type;
} t_msg_from_base;

typedef struct {
        t_msg_poste_type type;
        uint8_t          poste;
        uint8_t          vit_dest; // vitesse ou destination
        char             statut;   // statut du robot
} t_msg_from_robot;

extern uint8_t msg_base_received;

void init_com_poste(uint32_t baudrate);
void send_msg_to_base();
void send_msg_pave_to_base(message_pave *msg);
void base_msg_done();
void send_msg_IR_to_base(message_IR *msg);
void debug_write(const char *str);

typedef enum {
    ir_dispo       = 0x8,
    ir_enlevecolis = 0x4,
    ir_colispris   = 0x2,
    ir_livraison   = 0x1,
} etat_ir;

typedef enum {
    dispo       = 'D',
    enlevecolis = 'E',
    colispris   = 'C',
    livraison   = 'L',
} etat;