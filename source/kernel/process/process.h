#ifndef _PROCESS_H_
#define _PROCESS_H_


#include "stdint_shared.h"
#include "string_shared.h"

#include "context/ctx.h"
#include "hardware/scr/screen_driver.h"

#define PRC_CTXSWITCH_RATE_MS 1

typedef struct Process{
	char name[50];
	char * stack;
	Ctx * context;
	
	uint32_t sleep_start;
	uint32_t sleep_ms;
	
	ScreenInfo * screen;
}Process;

Process * prc_create(const char * name, uint32_t stackSize,
						uint32_t * entryPoint, Usermode mode);

void prc_startScheduler(void);

Process * prc_getCurrentProcess(void);

void prc_skipCurrentProc(void);
	
#endif