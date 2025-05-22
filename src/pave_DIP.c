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

#define ROW 5 // 1.18 1.19 1.20 1.21 1.22
#define COL 4 // 1.23 1.24 1.25 1.26
#define SIZE_REQUEST 16

uint8_t CPT_REQUEST = 0;

void init_matrix(void) {
    LPC_PINCON->PINSEL3 &= ~(0b11111 << 18); // Rows are config in outputs
    LPC_GPIO1->FIODIR   |= (0b1111 << 23);   // col are inputs

    LPC_GPIO1->FIODIR &= ~(1 << 23 | 1 << 24 | 1 << 25 | 1 << 26);
    LPC_GPIO1->FIODIR |= (0b11111 << 18);
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
                return '1';
            if (col_value & (1 << 1))
                return '2';
            if (col_value & (1 << 2))
                return '3';
            if (col_value & (1 << 3))
                return 'A';
        } else if (y == 1) {
            if (col_value & (1 << 0))
                return '4';
            if (col_value & (1 << 1))
                return '5';
            if (col_value & (1 << 2))
                return '6';
            if (col_value & (1 << 3))
                return 'B';
        } else if (y == 2) {
            if (col_value & (1 << 0))
                return '7';
            if (col_value & (1 << 1))
                return '8';
            if (col_value & (1 << 2))
                return '9';
            if (col_value & (1 << 3))
                return 'C';
        } else if (y == 3) {
            if (col_value & (1 << 0))
                return '*';
            if (col_value & (1 << 1))
                return '0';
            if (col_value & (1 << 2))
                return '#';
            if (col_value & (1 << 3))
                return 'D';
        }
    }

    return 0; // Retourne 0 si aucune touche n'est pressée
}

char *register_request(void) {
    char    request[16] = ""; // Tableau pour stocker les appuis (taille ajustable)
    static uint8_t index       = 0;  // Index pour suivre la position dans le tableau

    for (uint8_t row = 0; row < 4; row++) {
        char key = parsing_pave(row);

        if (key) {
            if (index == 0 && key != '#')
                continue;

            if (index < SIZE_REQUEST - 1) // Ajouter la touche si la taille le permet
                request[index++] = key;

            if (key == '*') {
                request[index] = '\0'; // Terminer la chaîne

                // Vérifier si la chaîne correspond au format attendu
                if (request[0] == '#' && index >= 4 && request[index - 1] == '*') {
                    return request; // Retourner la chaîne complète
                } else {
                    // Réinitialiser si le format est incorrect
                    index      = 0;
                    request[0] = '\0';
                }
            }
        }
    }

    return NULL; // Retourne NULL si la chaîne complète n'est pas encore formée
}

uint8_t DIP_switch() {
    char dip_switch[4] = "";
    uint8_t col_value = matrix(4); // Appeler matrix avec l'index 4 pour la 5ème ligne
    for (int i = 0; i < 4; i++)
        dip_switch[i] = (col_value & (1 << i)) ? '1' : '0'; // Convertir les bits en '1' ou '0'

    uint8_t dip_switch_value = 0;
    for (int i = 0; i < 4; i++)
        if (dip_switch[i] == '1')
            dip_switch_value |= (1 << (3 - i)); // Placer le bit correspondant

    return(dip_switch_value);
}