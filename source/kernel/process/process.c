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
#include "multitasking.h"

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
	prc->interruptions_count = 0;
	for (int i = 0; i < 6; i++)
		prc->interruptions_stat[i] = 0;
		
	for (int i = 0; i < 25; i++)
		prc->interruptions_stat_cpu[i] = 0;
	
	prc->sleep_until_new_messages = false;
	
	prc->context = calloc (sizeof(Ctx));
	
	// copy argument line
	if (arg_line != 0){
		int arg_line_size = (strlen((char*) arg_line) + 2) * sizeof(char);
		prc->arg_line = calloc(arg_line_size);
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
	if (!isMultitaskingInitialized())
		initMultitasking();
	
	addProcessToScheduler(prc);

	// set up events queue
	prc->list_msgs = list_new();
	
	_mem_init(prc->heap, heapSize * sizeof(char), 0);
		
	// synchrinization flag
	prc->sync_lock = FALSE;
	
	if (krn_is_init() == TRUE)
		krn_getIdleProcess()->sync_lock = FALSE;
	
	return prc;
}

bool prc_is_focused(){
	Process* prc = prc_getCurrentProcess();
	if (getFocusedProcessNode()->val == prc)
		return TRUE;
		
	return FALSE;
}

int idleInsStatus = 0;
void switchFocus(bool direct); 

/* return: 
	true: if idle process handle KYB event
	false: if KYB event should be delivired to focused app
*/
bool idleKeyboardEventHandler(KeyboardEvent event){
	if (event.key_code == 0x03){ //insert
		if (event.event_type == HW_KYB_EVENTTYPE_KEYPRESSED)
			idleInsStatus = 1;
		if (event.event_type == HW_KYB_EVENTTYPE_KEYRELEASED)
			idleInsStatus = 0;
	}
	
	if (event.key_code == '[' && idleInsStatus == 1){
		switchFocus(false);
		return true;
	}
	if (event.key_code == ']' && idleInsStatus == 1){
		switchFocus(true);
		return true;
	}
		
	return false;
}

void switchFocus(bool direct){
	Process* prc_current = (Process *)getFocusedProcessNode()->val;
	
	Process * prc;
	list_node_t * node;
	
	node = getFocusedProcessNode();
	
	if (node){
		if (direct == true){
			if (node->next != NULL)
				node = node->next;
			else
				node = firstTaskNode();
		} else {
			if (node->prev != NULL)
				node = node->prev;
			else
				node = lastTaskNode();
		}
		prc = (Process * )node->val;
		setFocusedProcess(prc, prc->screen->addr);
	}
}

/*
*	Messages queue
*/
void sendMessageToAll(PRC_MESSAGE type, int reason, int value){
	krn_getIdleProcess()->sync_lock = TRUE;
	Process * prc;
	list_node_t * node;
	
	list_iterator_t * it = list_iterator_new(getSchedullerList(), LIST_HEAD);
	while (node = list_iterator_next(it)){

		unsigned int msg = type << 24 | reason << 16 | value;
	
		prc = node->val;
		
		list_rpush(prc->list_msgs, list_node_new((void*)msg));

	}
	list_iterator_destroy(it);
	krn_getIdleProcess()->sync_lock = FALSE;
}

void sendMessageToFocused(PRC_MESSAGE type, int reason, int value){
	krn_getIdleProcess()->sync_lock = TRUE;
	Process * prc;
	
	unsigned int msg = type << 24 | reason << 16 | value;
	prc = (Process *)getFocusedProcessNode()->val;
	list_rpush(prc->list_msgs, list_node_new((void*)msg));

	krn_getIdleProcess()->sync_lock = FALSE;
}

/*
*	Multiprocessing draft. 
*/
bool isNeedSleep(Process * prc){
	if (prc->sleep_until_new_messages == true)
		if (prc->list_msgs->len == 0)
			return TRUE;
		else {
			prc->sleep_until_new_messages = false;
			return FALSE;
		}

	if (prc->sleep_ms == 0)
		return FALSE;
	
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
void clkAppCback(int clk){
	if (clk == KRN_APP_RESERVED_TIMER){
		sendMessageToAll(PRC_MESSAGE_CLK, 0, 0);
	}
}

void printPrcIntsStat(Process * prc){
	krn_debugLogf("%s - %d", prc->name, prc->interruptions_count);
	krn_debugLogf("    %s - %d", "CPU", prc->interruptions_stat[0]);
	krn_debugLogf("    %s - %d", "CLK", prc->interruptions_stat[1]);
	krn_debugLogf("    %s - %d", "SCR", prc->interruptions_stat[2]);
	krn_debugLogf("    %s - %d", "KYB", prc->interruptions_stat[3]);
	krn_debugLogf("    %s - %d", "NIC", prc->interruptions_stat[4]);
	krn_debugLogf("    %s - %d", "DKC", prc->interruptions_stat[5]);
	
	for (int i = 0; i < 25; i++)
		if (prc->interruptions_stat_cpu[i] != 0)
			krn_debugLogf("        %d - %d", i, prc->interruptions_stat_cpu[i]);
}

void switchProcToNext(){
	Process * prc;
	list_node_t * currProc = getCurrentProcessNode(); 
	int trys = getSchedullerList()->len + 2;
	do{
		
		prc = (Process *)nextProcess()->val;
		
		//printPrcIntsStat(prc);
		trys --;
						
	} while (isNeedSleep(prc) && trys > 0); // check sleep time
	//krn_debugLogf("active process: %s", prc->name);
}

bool isAllPrcAsleep(){
	list_node_t * prc_it = firstTaskNode();
	do {
		if (!isNeedSleep((Process *)prc_it->val))
			return false;
			
		prc_it = prc_it->next;
	} while (prc_it);
	
	return true;
}

void clkKrnCback(int clk){	
	if (clk != KRN_TIMER){
		return;	
	}
		
	/* if process need lock. skip scheduler
	
		Imitation of synchronization primitives.
		TODO: fix that
	*/
	if (((Process*)getCurrentProcessNode()->val)->sync_lock == TRUE
		|| krn_getIdleProcess()->sync_lock == TRUE
		|| getSchedullerList() == NULL 
		|| isAllPrcAsleep() == true){
		return;
	}

	switchProcToNext();
}

void prc_die(){
	Process* prc = prc_getCurrentProcess();
	prc->i_should_die = TRUE;
	
	// change focus
	if (getFocusedProcessNode()->val == prc){
		popFocusedStack();
		// prevous process should take a screen
		Process * focused_proc = (Process*)getFocusedProcessNode()->val;
		setFocusedProcess(focused_proc, focused_proc->screen->addr);
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
	hw_clk_setTimerCback(KRN_TIMER, clkKrnCback);
	hw_clk_setTimerCback(KRN_APP_RESERVED_TIMER, clkAppCback);
	hw_clk_setCountdownTimer(KRN_TIMER, PRC_CTXSWITCH_RATE_MS, true, true);

	while (TRUE){
		// is there messages to the kernel? 
		Process * prc = prc_getCurrentProcess();
		
		while (prc->list_msgs->len > 0){
			krn_getIdleProcess()->sync_lock = TRUE;
			list_node_t * node = list_lpop(prc->list_msgs);
			
			// TODO: should something be here?
			
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
	if (getCurrentProcessNode() && getCurrentProcessNode()->val)
		return (Process *)getCurrentProcessNode()->val;
	
	return NULL;
}

/*!
*	SUPEUSER MODE FUNC
*	TODO: it's hack
*/
void prc_skipCurrentProc(void){
	clkKrnCback(KRN_TIMER);
}