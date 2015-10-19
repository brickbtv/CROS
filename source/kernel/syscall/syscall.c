#include "syscall.h"

#include "process/process.h"

#include "hardware/clk/clock_driver.h"
#include "hardware/nic/network_driver.h"
#include "kernel/kernel_debug.h"

#include <details/memdetails.h>

//////////////////////
//	PROCESSES 		//
//////////////////////
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

void syscall_prc_haveNewMessages(void){
	Process * prc = prc_getCurrentProcess();
	if (prc->list_msgs != NULL)
		prc->context->gregs[0] = 1;
	else 
		prc->context->gregs[0] = 0;
}

void syscall_prc_getNextMessage(void){
	Process * prc = prc_getCurrentProcess();
	prc->context->gregs[0] = (unsigned int)prc->list_msgs->data;
	list_remove(&prc->list_msgs, prc->list_msgs);
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

F_SYSCALL syscalls_cbacks[] = 
{
	// processes 
	syscall_prc_sleep,
	syscall_prc_getCurrentProcessScreenInfo,
	syscall_prc_haveNewMessages,
	syscall_prc_getNextMessage,
	// clock
	syscall_clk_readTimeSinceBoot,
	syscall_clk_readCountdownTimer,
	syscall_clk_setCountdownTimer,
	// network
	syscall_nic_debug,
	syscall_nic_send,
	syscall_nic_recv
};