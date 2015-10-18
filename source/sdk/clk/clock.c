#include "clock.h"
#include "sdk/syscall_def.h"

unsigned int sdk_clk_timeSinceBoot(void){
	return app_syscall0(syscall_clk_readTimeSinceBoot);
}

unsigned int sdk_clk_readCountdownTimer(uint32_t timerNum){
	return app_syscall1(syscall_clk_readCountdownTimer, timerNum);
}

void sdk_clk_setCountdownTimer(uint32_t timerNum, uint32_t ms, bool autoReset){
	app_syscall3(syscall_clk_setCountdownTimer, timerNum, ms, autoReset);
}