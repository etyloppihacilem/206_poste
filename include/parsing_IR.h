#pragma once

#include <stdint.h>

void init_ir(void);
void parsing_message(char MESSAGE[], char DECODE[]);

extern uint8_t message_ready;