#include "process.h"

#include <kernel/kernel.h>
#include <stddef_shared.h>

#include "hardware/clk/clock_driver.h"
#include "containers/list.h"

// Processes loop.
list_node * listPrcLoop = NULL;
list_node * listPrcLoopLast = NULL;

static list_node * currProc;

/*!
*	Draft of multiprocessing stuff
*/
Process * prc_create(const char * name, uint32_t * stackTop,
						uint32_t * entryPoint, Usermode mode){
	Process * prc = malloc(sizeof(Process));
	memset(prc, 0, sizeof(Process));
	strcpy(prc->name, name);
	
	prc->context.gregs[CPU_REG_SP] = (uint32_t)stackTop;
	prc->context.gregs[CPU_REG_PC] = (uint32_t)entryPoint;
	prc->context.flags = mode;
	
	if (listPrcLoop == NULL){				// empty scheduler
		listPrcLoop = list_create(prc);
		listPrcLoop->next = listPrcLoop;	// loop itself
		listPrcLoopLast = listPrcLoop;
		currProc = listPrcLoop;
	} else {
		list_node * newPrc = list_create(prc);
		listPrcLoopLast->next = newPrc;
		listPrcLoopLast = newPrc;
		newPrc->next = listPrcLoop;			// loop to beginning
	}
	
	return prc;
}

/*!
*	Multiprocessing draft. 
*/
 void prc_ctxswitch(__reg("r0") void* ctx)
 INLINEASM("\t\
 ctxswitch [r0]");

void clkCback(int clk){	
	if (listPrcLoop == NULL){		// we don't have processes
		return;
	} else {
		currProc = currProc->next;
		Process * prc = currProc->data;
		prc_ctxswitch(&prc->context);
	}
}

/*!
*	Turn cpu into sleep. Wakes up only for processes switching.
*/
void prc_startScheduler(void){
	hw_clk_setTimerCback(KRN_TIMER, clkCback);
	hw_clk_setCountdownTimer(KRN_TIMER, PRC_CTXSWITCH_RATE_MS, true, true);

	while (TRUE){
		krn_halt();
	}
}