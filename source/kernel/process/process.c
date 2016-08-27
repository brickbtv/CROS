#include "process.h"

#include <kernel/kernel.h>
#include <stddef_shared.h>

#include "hardware/clk/clock_driver.h"
#include "hardware/cpu/cpu_driver.h"
#include "sdk/clk/clock.h"
#include "sdk/os/process.h"

#include "boot/boot.h"
#include "extern/tlsf/tlsf.h"
#include "stdcshared_defs.h"

// Processes loop.
list_t * listPrcLoop = NULL;
list_t * stackFocused = NULL;

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
						uint32_t * entryPoint, Usermode mode, uint32_t * arg_line, uint32_t * exist_canvas){
	if (krn_is_init() == TRUE)
		krn_getIdleProcess()->sync_lock = TRUE;
	
					 	
	Process * prc = calloc(sizeof(Process));
	//memset(prc, 0, sizeof(Process));
	
	// allocate stack
	prc->stack = calloc(stackSize * sizeof(char));
	prc->heap = calloc(heapSize * sizeof(char) + heapSize);
	// info
	strcpy(prc->name, name);
	prc->pid = totalPIDs++;
	
	prc->context = calloc (sizeof(Ctx));
	
	// copy argument line
	if (arg_line != 0){
		krn_debugLogf("arg line: %s",  (char*)arg_line);
		int arg_line_size = (strlen((char*) arg_line) + 2) * sizeof(char);
		prc->arg_line = calloc(arg_line_size);
		//memset(prc->arg_line, 0, arg_line_size);
		strcpy(prc->arg_line, (char*)arg_line);
	}
	
	// allocate screen buffer
	ScreenInfo info = hw_scr_screenInfo();
	
	prc->screen = calloc(sizeof(ScreenInfo));
	if (exist_canvas == NULL){
		prc->screen->addr = calloc(info.res_hor * info.res_ver * info.bytes_per_char*2);
		prc->screen->res_hor = info.res_hor;
		prc->screen->res_ver = info.res_ver;
		prc->screen->bytes_per_char = info.bytes_per_char;
		prc->screen->cur_x = 0;
		prc->screen->cur_y = 0;
		prc->exist_canvas = FALSE;
	} else {
		prc->screen = (ScreenInfo *)exist_canvas;
		prc->exist_canvas = TRUE;
	}
	
	
	
	// new process should take a screen
	hw_scr_mapScreenBuffer(prc->screen->addr);
	
	hw_scr_setTextColor(prc->screen, SCR_COLOR_GREEN);
	hw_scr_setBackColor(prc->screen, SCR_COLOR_BLACK);
	
	prc->i_should_die = FALSE;
	
	// sutup context stuffs
	prc->context->gregs[CPU_REG_SP] = (uint32_t)&prc->stack[stackSize - 1];
	prc->context->gregs[CPU_REG_PC] = (uint32_t)entryPoint;
	prc->context->flags = mode;
	
	prc->context->gregs[0] = (unsigned int) prc->arg_line;
	
	// insert process to scheduler
	if (listPrcLoop == NULL){				// empty scheduler
		listPrcLoop = list_new();
	}
	
	if (stackFocused == NULL){				// empty focused processes stack
		stackFocused = list_new();
	}
	
	list_rpush(stackFocused, list_node_new(prc));
	currFocusedProc = prc;
	

	// set up events queue
	prc->list_msgs = list_new();

	// set up first created process (idle) as current. needed in scheduler.
	if (currProc == NULL){
		currProc = list_rpush(listPrcLoop, list_node_new(prc));
	} else {
		list_rpush(listPrcLoop, list_node_new(prc));
	}
	
	_mem_init(prc->heap, heapSize * sizeof(char), 0);
		
	// synchrinization flag
	prc->sync_lock = FALSE;
	
	if (krn_is_init() == TRUE)
		krn_getIdleProcess()->sync_lock = FALSE;
	
	return prc;
}

void prc_initMessagesList(){
	//Process* prc = prc_getCurrentProcess();
	//prc->list_msgs = list_new();
}

bool prc_is_focused(){
	Process* prc = prc_getCurrentProcess();
	if (currFocusedProc == prc)
		return TRUE;
		
	return FALSE;
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

void sendMessageToFocused(PRC_MESSAGE type, int reason, int value){
	krn_getIdleProcess()->sync_lock = TRUE;
	Process * prc;
	list_node_t * node;
	
	list_iterator_t * it = list_iterator_new(listPrcLoop, LIST_HEAD);
	while (node = list_iterator_next(it)){
		// send to currently focused process
		if (currFocusedProc == node->val){		
			unsigned int msg = type << 24 | reason << 16 | value;
			prc = node->val;
			list_rpush(prc->list_msgs, list_node_new((void*)msg));
		}
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
		list_node_t * prev_prc;
		do{
			prev_prc = currProc;
			do {
				if (currProc->next != NULL){
					currProc = currProc->next;
				} else {
					currProc = listPrcLoop->head;
				}
				prc = currProc->val;
				// TODO: remove dead processes from loop
			} while (prc->i_should_die != 0);
			// kill process
			//if (((Process *)(prev_prc->val))->i_should_die == TRUE){
			//	list_remove(listPrcLoop, prev_prc);
			//}
			
		} while (isNeedSleep(prc)); // check sleep time
	}
}

void prc_die(){
	Process* prc = prc_getCurrentProcess();
	prc->i_should_die = TRUE;
	
	// change focus
	if (currFocusedProc == prc){
		list_rpop(stackFocused);
		currFocusedProc = list_tail(stackFocused)->val;
		// prevous process should take a screen
		hw_scr_mapScreenBuffer(currFocusedProc->screen->addr);
	}
	
	prc->sync_lock = FALSE;
	
	
	krn_getIdleProcess()->sync_lock = TRUE;
	
	if (prc->exist_canvas == FALSE){	
		free(prc->screen->addr);
		free(prc->screen);
	}
	
	free(prc->stack);
	_mem_destroy(prc->heap);
	free(prc->heap);
	
	krn_getIdleProcess()->sync_lock = FALSE;
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
			//free(node); 									// TODO: Heap corruption =( 
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
	if (currProc)
		return (Process *)currProc->val;
	
	return NULL;
}

/*!
*	SUPEUSER MODE FUNC
*	TODO: it's hack
*/
void prc_skipCurrentProc(void){
	clkCback(KRN_TIMER);
}