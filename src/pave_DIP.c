/* #####################################################################################################################

        _   ,_,   _         parsing_IR.c
       / `'=) (='` \        Created on 20 May. 2025 at 22h23
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/
#include "LPC17xx.h"
#include "com_super.h"
#include "sys/_intsup.h"
#include <stdint.h>
#include "pave_DIP.h"

#define ROW 5 // 1.18 1.19 1.20 1.21 1.22
#define COL 4 // 1.23 1.24 1.25 1.26
#define SIZE_REQUEST 16
#define LENGTH_FIFO_PAVE 16

uint8_t CPT_REQUEST = 0;
uint8_t rebond      = 0;

void init_matrix(void) {
    LPC_PINCON->PINSEL3 &= ~(0b11111 << 18); // Rows are config in outputs
    LPC_GPIO1->FIODIR   |= (0b1111 << 23);   // col are inputs

    LPC_GPIO1->FIODIR &= ~(1 << 23 | 1 << 24 | 1 << 25 | 1 << 26);
    LPC_GPIO1->FIODIR |= (0b11111 << 18);

    LPC_SC->PCONP |= (1 << 23);           // Activer TIMER3
    LPC_TIM3->PR   = 0;                   // Pas de prescaler
    LPC_TIM3->MR0  = 25000000 / 5;        // 200 ms (25 MHz / 5 Hz)
    LPC_TIM3->MCR  = (1 << 0) | (1 << 1); // Interruption et reset sur MR0
    LPC_TIM3->TCR  = (1 << 1);            // Réinitialiser TIMER3
    LPC_TIM3->TCR  = (1 << 0);            // Activer TIMER3

    NVIC_EnableIRQ(TIMER3_IRQn); // Activer l'interruption TIMER3
}

void TIMER3_IRQHandler(void) {
    LPC_TIM3->IR = (1 << 0); // Effacer le drapeau d'interruption
    rebond       = 1;        // Autoriser l'enregistrement de nouvelles touches
}

uint8_t matrix(uint8_t y) {
    if (y >= ROW)
        return 0;

    uint32_t ret       = 1 << (18 + y);
    LPC_GPIO1->FIOPIN &= ~ret; // select the line to read

    while (LPC_GPIO1->FIOPIN & ret)
        ;

    ret                = (LPC_GPIO1->FIOPIN & (1 << 23 | 1 << 24 | 1 << 25 | 1 << 26)) >> 23;
    LPC_GPIO1->FIOPIN |= 0b11111 << 18; // on remet la ligne niveau haut
    return (~ret) & 0xF;
}

uint8_t parsing_pave(uint8_t y) {
    if (y >= ROW - 1)
        return 0; // Retourne 0 si la ligne demandée est invalide

    uint8_t col_value = matrix(y); // Lire l'état des colonnes pour la ligne donnée
    if (col_value) {               // Si une touche est pressée
        if (y == 0) {
            if (col_value & (1 << 0))
                return 1;
            if (col_value & (1 << 1))
                return 2;
            if (col_value & (1 << 2))
                return 3;
            if (col_value & (1 << 3))
                return 'A';
        } else if (y == 1) {
            if (col_value & (1 << 0))
                return 4;
            if (col_value & (1 << 1))
                return 5;
            if (col_value & (1 << 2))
                return 6;
            if (col_value & (1 << 3))
                return 'B';
        } else if (y == 2) {
            if (col_value & (1 << 0))
                return 7;
            if (col_value & (1 << 1))
                return 8;
            if (col_value & (1 << 2))
                return 9;
            if (col_value & (1 << 3))
                return 'C';
        } else if (y == 3) {
            if (col_value & (1 << 0))
                return '*';
            if (col_value & (1 << 1))
                return 0;
            if (col_value & (1 << 2))
                return '#';
            if (col_value & (1 << 3))
                return 'D';
        }
    }

    return 0; // Retourne 0 si aucune touche n'est pressée
}


message_pave FIFO_pave[LENGTH_FIFO_PAVE];
uint8_t      FIFO_r = 0;
uint8_t      FIFO_w = 0;

void register_request(void) {
    static char    request[SIZE_REQUEST];
    static uint8_t idex     = 0;
    static char    last_key = 0;

    if (!rebond)
        return;

    char key = 0;
    for (uint8_t row = 0; row < 4; row++) {
        key = parsing_pave(row);
        if (key)
            break;
    }

    if (!key || key == last_key)
        return; // Rien ou même touche que la dernière

    last_key      = key;
    rebond        = 0;        // bloque la saisie
    LPC_TIM3->TCR = (1 << 1); // reset timer
    LPC_TIM3->TCR = (1 << 0); // start

    if (idex == 0 && key != '#')
        return; // doit commencer avec #

    if (idex < SIZE_REQUEST - 1)
        request[idex++] = key;

    if (key == '*') { // fin request
        request[idex] = '\0';
        if (request[0] == '#' && idex >= 5 && request[idex - 1] == '*') {

            FIFO_pave[FIFO_w].ID_post_dix = request[1];
            FIFO_pave[FIFO_w].ID_post_uni = request[2]; // Extraire ID_post
            FIFO_pave[FIFO_w].livreur     = request[3]; // Extraire livreur

            if (FIFO_w >= LENGTH_FIFO_PAVE - 1)
                FIFO_w = 0;
            else
                FIFO_w++;

            idex     = 0; // Réinitialiser pour la prochaine requête
            last_key = 0;
        } else {
            // Requête invalide, réinitialiser
            idex       = 0;
            request[0] = '\0';
        }
    }
}

uint8_t DIP_switch() {
    char    dip_switch[4] = "";
    uint8_t col_value     = matrix(4); // Appeler matrix avec l'index 4 pour la 5ème ligne
    for (int i = 0; i < 4; i++)
        dip_switch[i] = (col_value & (1 << i)) ? '1' : '0'; // Convertir les bits en '1' ou '0'

    uint8_t dip_switch_value = 0;
    for (int i = 0; i < 4; i++)
        if (dip_switch[i] == '1')
            dip_switch_value |= (1 << (3 - i)); // Placer le bit correspondant

    return dip_switch_value;
}

/*
 * returns message to treat, or NULL
 * */
message_pave *get_pave_msg() {
    message_pave *msg_pave;

    if (FIFO_r == FIFO_w)
        msg_pave = 0;
    else
        msg_pave = FIFO_pave + FIFO_r; // on retourne un pointeur sur le message

    return msg_pave;
}

/*
 * call this when done with processing message.
 * */
void ir_pave_msg_done() {

    if (FIFO_r == LENGTH_FIFO_PAVE - 1)
        FIFO_r = 0;
    else
        FIFO_r++;
}