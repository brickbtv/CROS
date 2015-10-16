#include "clock.h"
#include "kernel/syscall/syscall.h"

unsigned int sdk_clk_TimeSinceBoot(void){
	return app_syscall0((int)syscall_clk_readTimeSinceBoot);
}