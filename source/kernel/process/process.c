#include "process.h"

#include <kernel/kernel.h>
#include <stddef_shared.h>

#include "hardware/clk/clock_driver.h"
#include "containers/list.h"
#include "sdk/clk/clock.h"
#include "sdk/os/process.h"

// Processes loop.
list_node * listPrcLoop = NULL;
list_node * listPrcLoopLast = NULL;

static list_node * currProc;
static Process * currFocusedProc;

// TODO: leave here only superuser mode code

/*!
*	Draft of multiprocessing stuff
*/
Process * prc_create(const char * name, uint32_t stackSize,
						uint32_t * entryPoint, Usermode mode){
	Process * prc = malloc(sizeof(Process));
	memset(prc, 0, sizeof(Process));
	
	// allocate stack
	prc->stack = malloc(stackSize * sizeof(char));
	
	// process name
	// TODO: process pid
	strcpy(prc->name, name);
	
	prc->context = malloc (sizeof(Ctx));
	
	// allocate screen buffer
	ScreenInfo info = hw_scr_screenInfo();
	
	prc->screen = malloc(sizeof(ScreenInfo));
	prc->screen->addr = malloc(info.res_hor * info.res_ver * info.bytes_per_char*2);
	prc->screen->res_hor = info.res_hor;
	prc->screen->res_ver = info.res_ver;
	prc->screen->bytes_per_char = info.bytes_per_char;
	
	hw_scr_setTextColor(prc->screen, SCR_COLOR_GREEN);
	hw_scr_setBackColor(prc->screen, SCR_COLOR_BLACK);
	
	currFocusedProc = prc;
	
	// sutup context stuffs
	prc->context->gregs[CPU_REG_SP] = (uint32_t)&prc->stack[stackSize - 1];
	prc->context->gregs[CPU_REG_PC] = (uint32_t)entryPoint;
	prc->context->flags = mode;
	
	// insert process to scheduler
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
	
	// clean msgs queue
	prc->list_msgs = NULL;
	
	return prc;
}

/*
*	Messages queue
*/
void sendMessageToAll(PRC_MESSAGE type, int reason, int value){
	Process * prc;
	list_node * it = listPrcLoop;
	
	PrcMessage * msg = malloc(sizeof(PrcMessage));
	msg->type = type;
	msg->reason = reason;
	msg->value = value;
	
	do{
		prc = it->data;
		if (prc->list_msgs == NULL){
			prc->list_msgs = list_create((void*)msg);
		} else {
			list_insert_end(prc->list_msgs, (void*)msg);
		}
		it = it->next;
	} while (listPrcLoop != it); 
}

/*
*	Multiprocessing draft. 
*/
bool isNeedSleep(Process * prc){
	if (prc->sleep_ms == 0){
		return FALSE;
	}
	
	uint32_t curTime = hw_clk_readTimeSinceBoot();
	
	if (prc->sleep_start + prc->sleep_ms < curTime){
		prc->sleep_ms = 0;
		return FALSE;
	} 
	
	return TRUE;	
}

 void prc_ctxswitch(__reg("r0") void* ctx)
 INLINEASM("\t\
 ctxswitch [r0]");

void clkCback(int clk){	
	if (clk != KRN_TIMER)
		return;

	if (listPrcLoop == NULL){		// we don't have processes
		return;
	} else {
		Process * prc;
		do{
			currProc = currProc->next;
			prc = currProc->data;
		} while (isNeedSleep(prc)); // check sleep time
			
		//prc_ctxswitch(prc->context);
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

/*!
*	Can be useful to recieve current context
*/
Process * prc_getCurrentProcess(void){
	return (Process *)currProc->data;
}

/*!
*	SUPEUSER MODE FUNC
*/
void prc_skipCurrentProc(void){
	clkCback(KRN_TIMER);
}