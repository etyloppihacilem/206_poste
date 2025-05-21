/* #####################################################################################################################

        _   ,_,   _         parsing_IR.c
       / `'=) (='` \        Created on 20 May. 2025 at 22h23
      /.-.-.\ /.-.-.\       by Amaury Jacob
      `      "      `

#####################################################################################################################
*/
#include "LPC17xx.h"

#define ROW 5 // 1.18 1.19 1.20 1.21 1.22 
#define COL 4 // 1.23 1.24 1.25 1.26


void init_matrix(void)
{
      LPC_GPIO1->FIODIR |= (0b11111<<18); //Rows are config in input
      LPC_GPIO1->FIODIR0 &= ~(0b1111<<23); //col are outputs
      
      LPC_GPIO1->FIODIR |= (0b11111<<18);


}

void matrix(uint16_t y)
{
      if(y >= ROW) return;

      uint32_t ret = 1 << (18 + y); 
      LPC_GPIO0->FIOPIN &= ~ret;  //select the line to read
      
      while (LPC_GPIO0->FIOPIN & ret);

      ret = (LPC_GPIO0->FIOPIN & (1 << 23 | 1 << 24 | 1 << 25 | 1<<26)) >> 23;
      LPC_GPIO0->FIOPIN |= 0b11111 << 18; // on remet la ligne niveau haut
      return (~ret) & 0xF;   
}


