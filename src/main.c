#include <LPC17xx.h>
#include "parsing_IR.c"
#include "pave_DIP.c"

int main(void)
{
    init_ir();
    init_matrix();
    

    if (message_ready)
    {
        parcing_messga(MESSAGE, DECODE);
        message_ready = 0;
    }
}
