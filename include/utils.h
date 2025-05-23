/* #####################################################################################################################

               """          utils.h
        -\-    _|__
         |\___/  . \        Created on 19 May. 2025 at 23:09
         \     /(((/        by hmelica
          \___/)))/         hmelica@student.42.fr

##################################################################################################################### */

#pragma once

#include <stdint.h>

uint8_t is_livraison(char c);
uint8_t is_state(char c);
uint8_t parse_nb(char a, char b);
uint8_t cote_depot(char c);
uint8_t cote_reception(char c);
uint8_t parse_hex(char c);
