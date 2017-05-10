#ifndef _ASM_DECODER_H_
#define _ASM_DECODER_H_

#include "sdk/scr/screen.h"

void init_symbol_table();
unsigned char * decode_instruction(Canvas * canvas, unsigned char * caddr);

#endif