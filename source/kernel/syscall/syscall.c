#include "syscall.h"

#include "process/process.h"

#include "hardware/clk/clock_driver.h"
#include "hardware/nic/network_driver.h"
#include "kernel/kernel_debug.h"

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
	
	krn_debugLogf("sdads %s %d", msg, size);
	
	hw_nic_bufferOutgoingPacket(0, msg, size);
}

//////////////////////

F_SYSCALL syscalls_cbacks[] = 
{
	// processes 
	syscall_prc_sleep,
	syscall_prc_getCurrentProcessScreenInfo,
	// clock
	syscall_clk_readTimeSinceBoot,
	syscall_clk_readCountdownTimer,
	syscall_clk_setCountdownTimer,
	// network
	syscall_nic_debug
};