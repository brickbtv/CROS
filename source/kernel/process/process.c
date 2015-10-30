#include "process.h"

#include <kernel/kernel.h>
#include <stddef_shared.h>

#include "hardware/clk/clock_driver.h"
#include "hardware/cpu/cpu_driver.h"
#include "containers/list.h"
#include "sdk/clk/clock.h"
#include "sdk/os/process.h"

#include "mmu/mmu.h"

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
		
		HeapInfo heapInfo;
		bool ret = prc_setupMemory(
					prc, KERNEL_STACKSIZE, KERNEL_HEAPSIZE, (SIZE_TO_PAGES(hw_cpu_retRamAmount())*4),
					&heapInfo);
		stdcshared_init((DebugLogFunc)memCback, 
						(void*)heapInfo.start, 
						heapInfo.size);
	} else {
		list_rpush(listPrcLoop, list_node_new(prc));
		HeapInfo heapInfo;
		bool ret = prc_setupMemory(
					prc, stackSize, heapSize, 0,
					&heapInfo);
		
		if (ret == 0){
			krn_debugBSODf("Process Creation", "prc_setupMemory failed");
		}
	}
	
	// clean msgs queue
	prc->list_msgs = list_new();
	
	// synchrinization flag
	prc->sync_lock = FALSE;
	
	return prc;
}

bool prc_setupMemory(Process * prc, unsigned int stackSize, 
									unsigned int heapSize,
									unsigned int extraSize,
									HeapInfo * heapInfo){
	bool isKrn = (prc->pid == PID_KERNEL);
									
	int sharedSize = align(processInfo.sharedReadWriteSize, 4);	
	stackSize = align( (stackSize > PRC_MIN_STACKSIZE) ? stackSize:PRC_MIN_STACKSIZE, 4);
	heapSize = align(heapSize,4);
	size_t neededSize = stackSize + extraSize + sharedSize + heapSize;
	// If no dynamic memory needed, then don't waste space with tlsf
	if (heapSize)
		neededSize += tlsf_size();
	neededSize = align(neededSize, MMU_PAGE_SIZE);
	int numPages = SIZE_TO_PAGES(neededSize);
	
	//
	// Calculate first page
	//
	int firstPage;
	if (isKrn) {
		firstPage = ADDR_TO_PAGE( align(
			processInfo.sharedReadWriteAddr+sharedSize,MMU_PAGE_SIZE));
	} else {
		firstPage = mmu_findFreePages( numPages );
		if (firstPage==-1) {
			krn_debugLog("Not enough contiguous pages to create process heap");
			return FALSE;
		}
	}
	
	uint8_t* memStart = PAGE_TO_ADDR(firstPage);
	prc->firstPage = firstPage;
	prc->numPages = numPages;
	prc->ds = (unsigned int)(memStart + stackSize + extraSize);
	
	
	// WARNING: This needs to be setup before initializing the MMU, otherwise
	// if this is the kernel process initialization, it will crash since this
	// thread context is in protected memory
	//TCB* tcb = pcb->mainthread;
	prc->context->gregs[CPU_REG_DS] = (uint32_t)prc->ds;	
	
	//
	// Stack
	// The stack is at the start, to help catch stack overflow,
	// since it will crash when trying to write to the lower page
	// Note: Not actually setting the stack register here, because this code
	// is also used to initialize the kernel process. The caller is responsible
	// by setting the registers
	uint8_t* ptr = memStart;
	prc->stackBottom = (unsigned int)ptr;
	prc->stackTop = (unsigned int)(ptr + stackSize);

	// If this is the kernel process, we initialize the mmu as soon as
	// possible to help catch bugs during kernel boot.		
	if (isKrn)
	{
		//assert(extraSize>=MMU_TABLE_SIZE);
		mmu_init(firstPage, numPages, memStart+stackSize);
	}
	
	//
	// Shared data
	//
	// Temporarly set write permissions for the kernel so we can write stuff into
	// the process heap area
	/*prc_setPagesAccess(pcb, PID_KERNEL);
	memcpy(pcb->ds , (void*)processInfo.sharedReadWriteAddr, sharedSize);
	ptr = (uint8_t*)pcb->ds + sharedSize;
	prc_setPagesAccess(pcb, pcb->info.pid);*/

	memset(heapInfo, 0, sizeof(*heapInfo));
	if (heapSize) {
		heapInfo->start = (unsigned int)ptr;
		heapInfo->size = neededSize - (ptr-memStart);
	}

	//pcb->info.memory = mmu_countPages(pcb->info.pid) * MMU_PAGE_SIZE;
	//krn.info.mem_available = mmu_countPages(PID_NONE)*MMU_PAGE_SIZE;
	return TRUE;
}

void prc_setPagesAccess(Process * prc, uint8_t pid)
{
	mmu_setPages(prc->firstPage, prc->numPages, pid, true, true, false);
	
	mmu_setPages(
		ADDR_TO_PAGE(prc->stackBottom),
		SIZE_TO_PAGES(prc->stackTop - prc->stackBottom),
		pid, true, true, false);
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