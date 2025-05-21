/* #####################################################################################################################

               """          com_super.h
        -\-    _|__
         |\___/  . \        Created on 16 May. 2025 at 14:35
         \     /(((/        by hmelica
          \___/)))/         hmelica@student.42.fr

##################################################################################################################### */

#pragma once

#include <stdint.h>
#include <stdio.h>

void init_com_super(uint32_t baudrate);

int fputc(int c, FILE *f); // needed for printf, do not use this
int _write(int file, char *ptr, int len);
