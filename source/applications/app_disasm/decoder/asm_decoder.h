#ifndef _ASM_DECODER_H_
#define _ASM_DECODER_H_

#include "sdk/scr/screen.h"
#include "stdlib/containers/list.h"

typedef struct STR_SYMBOL{
	unsigned int address;
	char name[64];
} STR_SYMBOL;

list_t * init_symbol_table();
unsigned char * decode_instruction(Canvas * canvas, unsigned char * caddr, list_t * sym_table);

#endif