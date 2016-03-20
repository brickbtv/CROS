#include "process.h"

#include <kernel/kernel.h>
#include <stddef_shared.h>

#include "hardware/clk/clock_driver.h"
#include "hardware/cpu/cpu_driver.h"
#include "containers/list.h"
#include "sdk/clk/clock.h"
#include "sdk/os/process.h"

#include "boot/boot.h"
#include "extern/tlsf/tlsf.h"
#include "stdcshared_defs.h"

// Processes loop.
list_t * listPrcLoop = NULL;

static list_node_t * currProc = NULL;
static Process * currFocusedProc;

// TODO: leave here only superuser mode code

static unsigned int totalPIDs = 1;

bool prc_setupMemory(Process * prc, unsigned int stackSize, 
									unsigned int heapSize,
									unsigned int extraSize,
									HeapInfo * heapInfo);

/*!
*	Draft of multiprocessing stuff
*/
Process * prc_create(const char * name, uint32_t stackSize, uint32_t heapSize,
						uint32_t * entryPoint, Usermode mode){
	Process * prc = malloc(sizeof(Process));
	memset(prc, 0, sizeof(Process));
	
	// allocate stack
	prc->stack = malloc(stackSize * sizeof(char));
	
	// info
	strcpy(prc->name, name);
	prc->pid = totalPIDs++;
	
	prc->context = malloc (sizeof(Ctx));
	
	// allocate screen buffer
	ScreenInfo info = hw_scr_screenInfo();
	
	prc->screen = malloc(sizeof(ScreenInfo));
	prc->screen->addr = info.addr;//malloc(info.res_hor * info.res_ver * info.bytes_per_char*2);
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
		listPrcLoop = list_new();
	}
	
	// set up first created process (idle) as current. needed in scheduler.
	if (currProc == NULL){
		currProc = list_rpush(listPrcLoop, list_node_new(prc));
		
	} else {
		list_rpush(listPrcLoop, list_node_new(prc));
		HeapInfo heapInfo;
	}
	
	// clean msgs queue
	prc->list_msgs = list_new();
	
	// synchrinization flag
	prc->sync_lock = FALSE;
	
	return prc;
}

/*
*	Messages queue
*/
void sendMessageToAll(PRC_MESSAGE type, int reason, int value){
	krn_getIdleProcess()->sync_lock = TRUE;
	Process * prc;
	list_node_t * node;
	
	list_iterator_t * it = list_iterator_new(listPrcLoop, LIST_HEAD);
	while (node = list_iterator_next(it)){
		/*PrcMessage * msg = malloc(sizeof(PrcMessage));
		msg->type = type;
		msg->reason = reason;
		msg->value = value;*/
		unsigned int msg = type << 24 | reason << 16 | value;
	
		prc = node->val;
		
		list_rpush(prc->list_msgs, list_node_new((void*)msg));
			
		/*size_t us, fr, ma;
		_getmemstats(&us, &fr, &ma);
		krn_debugLogf("us: %d, fr: %d, ma: %d\n", us, fr, ma);*/
	}
	list_iterator_destroy(it);
	krn_getIdleProcess()->sync_lock = FALSE;
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

/*!
*	Timing processing
*/
void clkCback(int clk){	
	if (clk != KRN_TIMER)
		return;
		
	/* if process need lock. skip scheduler
	
		Imitation of synchronization primitives.
		TODO: fix that
	*/
	if (((Process*)currProc->val)->sync_lock == TRUE){
		return;
	}
	
	// kernel lock
	if (krn_getIdleProcess()->sync_lock == TRUE){
		return;
	}
		
	if (listPrcLoop == NULL){		// we don't have any processes
		return;
	} else {
		Process * prc;
		do{
			if (currProc->next != NULL){
				currProc = currProc->next;
			} else {
				currProc = listPrcLoop->head;
			}
			prc = currProc->val;
		} while (isNeedSleep(prc)); // check sleep time
	}
}

/*!
*	Turn cpu into sleep. Wakes up only for processes switching.
*/
void prc_startScheduler(void){
	hw_clk_setTimerCback(KRN_TIMER, clkCback);
	hw_clk_setCountdownTimer(KRN_TIMER, PRC_CTXSWITCH_RATE_MS, true, true);

	while (TRUE){
		// is there messages to the kernel? 
		Process * prc = prc_getCurrentProcess();
		
		while (prc->list_msgs->len > 0){
			krn_getIdleProcess()->sync_lock = TRUE;
			list_node_t * node = list_lpop(prc->list_msgs);
			//PrcMessage * msg = (PrcMessage * )node->val; // TODO: process it. do not just destroy 
			free(node); 
			krn_getIdleProcess()->sync_lock = FALSE;
		}
		
		// wait for interruptions
		// save the energy!
		krn_halt();
	}
}

/*!
*	Can be useful to recieve current context
*/
Process * prc_getCurrentProcess(void){
	return (Process *)currProc->val;
}

/*!
*	SUPEUSER MODE FUNC
*	TODO: it's hack
*/
void prc_skipCurrentProc(void){
	clkCback(KRN_TIMER);
}