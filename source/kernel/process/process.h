#ifndef _PROCESS_H_
#define _PROCESS_H_


#include "stdint_shared.h"
#include "string_shared.h"

#include "context/ctx.h"

typedef struct Process{
	char name[50];
	Ctx context;
}Process;

Process prc_create(const char * name, uint32_t * stackTop,
						uint32_t * entryPoint, Usermode mode);

#endif