#include "syscall.h"

#include "process/process.h"

#include "hardware/clk/clock_driver.h"
#include "hardware/nic/network_driver.h"
#include "hardware/dkc/disk_driver.h"
#include "kernel/kernel_debug.h"
#include "kernel/kernel.h"

#include <details/memdetails.h>

//////////////////////
//	PROCESSES 		//
//////////////////////
void syscall_prc_create_process(void){
	krn_getIdleProcess()->sync_lock = TRUE;

	Process * prc = prc_getCurrentProcess();
	
	uint32_t entry_point = prc->context->gregs[0];	
	uint32_t arg_line = prc->context->gregs[1];	
	uint32_t * exist_canvas = (uint32_t *)prc->context->gregs[2];	
	
	prc_create("app_test", 1024*10, 1024*20, (uint32_t*)entry_point, USERMODE_USER, (uint32_t *)arg_line, exist_canvas);
	
	
	krn_getIdleProcess()->sync_lock = FALSE;
}

void syscall_prc_sleep(void){
	Process * prc = prc_getCurrentProcess();
	
	uint32_t ms = prc->context->gregs[0];	
	
	prc->sleep_start = hw_clk_readTimeSinceBoot();
	prc->sleep_ms = ms;
}

void syscall_prc_getCurrentProcessScreenInfo(void){
	Process * prc = prc_getCurrentProcess();
	prc->context->gregs[0] = (unsigned int)prc->screen;	
}

void syscall_prc_getCurrentProcessHeapPointer(void){
	Process * prc = prc_getCurrentProcess();
	if (prc)
		prc->context->gregs[0] = (unsigned int)prc->heap;	
}

void syscall_prc_haveNewMessages(void){
	Process * prc = prc_getCurrentProcess();
	if (prc->list_msgs->len > 0)
		prc->context->gregs[0] = 1;
	else 
		prc->context->gregs[0] = 0;
}

void syscall_prc_getNextMessage(void){
	krn_getIdleProcess()->sync_lock = TRUE;
	Process * prc = prc_getCurrentProcess();
	list_node_t * node = list_lpop(prc->list_msgs);
	//PrcMessage * msg = (PrcMessage*)node->val;
	unsigned int msg = (unsigned int)node->val;
	prc->context->gregs[0] = msg;//(unsigned int)node->val;
	//if (node)
	//	free(node);
	krn_getIdleProcess()->sync_lock = FALSE;
}

void syscall_prc_lock(void){
	Process * prc = prc_getCurrentProcess();
	prc->sync_lock = TRUE;
}

void syscall_prc_unlock(void){
	Process * prc = prc_getCurrentProcess();
	prc->sync_lock = TRUE;
}

void syscall_prc_is_focused(void){
	Process * prc = prc_getCurrentProcess();
	
	if (prc_is_focused())
		prc->context->gregs[0] = 1;
	else 
		prc->context->gregs[0] = 0;
}

void syscall_prc_die(void){
	prc_die();
}

//////////////////////
//		CLOCK 		//
//////////////////////
void syscall_clk_readTimeSinceBoot(void){
	Process * prc = prc_getCurrentProcess();
	prc->context->gregs[0] = hw_clk_readTimeSinceBoot();
}

void syscall_clk_readCountdownTimer(void){
	Process * prc = prc_getCurrentProcess();
	uint32_t timerNum = prc->context->gregs[0];
	prc->context->gregs[0] = hw_clk_readCountdownTimer(timerNum);
}

void syscall_clk_setCountdownTimer(void){
	Process * prc = prc_getCurrentProcess();
	uint32_t timerNum = prc->context->gregs[0];
	uint32_t ms = prc->context->gregs[1];
	bool auto_reset = (bool)prc->context->gregs[2];
	
	hw_clk_setCountdownTimer(timerNum, ms, auto_reset, TRUE);
}

//////////////////////
//		NETWORK		//
//////////////////////

void syscall_nic_debug(void){
	Process * prc = prc_getCurrentProcess();
	char * msg = (char *)prc->context->gregs[0];
	uint32_t size = prc->context->gregs[1];
		
	hw_nic_bufferOutgoingPacket(0, msg, size);
}

void syscall_nic_send(void){
	Process * prc = prc_getCurrentProcess();
	
	uint32_t addr = prc->context->gregs[0];
	char * msg = (char *)prc->context->gregs[1];
	uint32_t size = prc->context->gregs[2];
	
	hw_nic_bufferOutgoingPacket(addr, msg, size);
}

void syscall_nic_recv(void){
	Process * prc = prc_getCurrentProcess();
	
	char * msg = (char *)prc->context->gregs[0];
	int max_size = prc->context->gregs[1];
	int * addr = (int *)prc->context->gregs[2];
	int recv_size;
	hw_nic_retrieveIncomingPacket(msg, max_size, addr, &recv_size);
	// TODO: return all args
	// TODO: process errors
	// TODO: replace ints by messages
	// TODO: implement all TODO: labels
	prc->context->gregs[0] = recv_size;
}

//////////////////////
//	DISK DRIVE		//
//////////////////////

void syscall_dkc_read(void){
	Process * prc = prc_getCurrentProcess();
	
	u32 diskNum = prc->context->gregs[0];
	u32 sectorNum = prc->context->gregs[1];
	void * data = (void*)prc->context->gregs[2];
	int size = prc->context->gregs[3];
	
	
	hw_dkc_read_sync(diskNum, sectorNum, data, size);
}

void syscall_dkc_write(void){
	Process * prc = prc_getCurrentProcess();
	
	u32 diskNum = prc->context->gregs[0];
	u32 sectorNum = prc->context->gregs[1];
	void * data = (void *)prc->context->gregs[2];
	int size = prc->context->gregs[3];
	
	hw_dkc_write_sync(diskNum, sectorNum, data, size);
}

void syscall_dkc_getInfo(void){
	Process * prc = prc_getCurrentProcess();
	int diskNum = prc->context->gregs[0];
	DiskQuery * dq = (DiskQuery *)prc->context->gregs[1];
	*dq = hw_dkc_query(diskNum);
}

void syscall_dkc_isReady(void){
	Process * prc = prc_getCurrentProcess();
	int diskNum = prc->context->gregs[0];
	prc->context->gregs[0] = hw_dkc_is_ready(diskNum);
}

//////////////////////

F_SYSCALL syscalls_cbacks[] = 
{
	// processes 
	syscall_prc_create_process,
	syscall_prc_sleep,
	syscall_prc_getCurrentProcessScreenInfo,
	syscall_prc_getCurrentProcessHeapPointer,
	syscall_prc_haveNewMessages,
	syscall_prc_getNextMessage,
	syscall_prc_lock,
	syscall_prc_unlock,
	syscall_prc_is_focused,
	syscall_prc_die,
	// clock
	syscall_clk_readTimeSinceBoot,
	syscall_clk_readCountdownTimer,
	syscall_clk_setCountdownTimer,
	// network
	syscall_nic_debug,
	syscall_nic_send,
	syscall_nic_recv,
	// disk drive
	syscall_dkc_read,
	syscall_dkc_write,
	syscall_dkc_getInfo,
	syscall_dkc_isReady
};