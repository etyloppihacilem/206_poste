#pragma once

#include <stdint.h>

typedef enum {
    p_vide         = 0,
    EOT            = 'N',
} t_msg_poste_type;

typedef struct {
        t_msg_poste_type type;
} t_msg_from_base;


typedef struct {
    t_msg_poste_type type;
    uint8_t          poste;
    uint8_t          vit_dest;  // vitesse ou destination
    char             statut;    // statut du robot
} t_msg_from_robot;

void init_com_poste(uint32_t baudrate);
void base_msg_done();
