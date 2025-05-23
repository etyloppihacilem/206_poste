/* #####################################################################################################################

               """          utils.c
        -\-    _|__
         |\___/  . \        Created on 19 May. 2025 at 23:09
         \     /(((/        by hmelica
          \___/)))/         hmelica@student.42.fr

##################################################################################################################### */

#include <stdint.h>

uint8_t is_number(char c) {
    return '0' <= c && c <= '9';
}

uint8_t is_livraison(char c) {
    return 'A' <= c && c <= 'D';
}

/*
 * Tableau des livraisons et des cotés :
 *
 *    | depot | reception
 * ---|-------|-----------
 *  A |   D   |     D
 *  B |   D   |     G
 *  C |   G   |     G
 *  D |   G   |     D
 *
 *  0 pour D droite
 *  1 pour G gauche
 *
 * */

uint8_t cote_depot(char c) {
    // retourne 0 pour droite, 1 pour gauche
    if (c == 'A' || c == 'B')
        return 0;
    return 1;
}

uint8_t cote_reception(char c) {
    // retourne 0 pour droite, 1 pour gauche
    if (c == 'A' || c == 'D')
        return 0;
    return 1;
}

uint8_t is_state(char c) {
    return ('C' <= c && c <= 'E') || c == 'L';
}

// parse number ab
uint8_t parse_nb(char a, char b) {
    if (!is_number(a) || !is_number(b))
        return -1; // 255, ce qui est toujours plus grand que le nombre de robots ou de postes.
    return (a - '0') * 10 + (b - '0');
}

uint8_t parse_hex(char c) {
    if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
    if ('0' <= c && c <= '9')
        return c - '0';
    else
        return 0;
}
