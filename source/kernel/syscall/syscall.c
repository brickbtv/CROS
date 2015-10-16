#include "syscall.h"

#include "process/process.h"

#include "hardware/clk/clock_driver.h"

void syscall_clk_readTimeSinceBoot(void){
	Process * prc = prc_getCurrentProcess();
	prc->context.gregs[0] = hw_clk_readTimeSinceBoot();
}
