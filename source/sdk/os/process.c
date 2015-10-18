#include "process.h"
#include "sdk/syscall_def.h"

unsigned int sdk_prc_sleep(unsigned int ms){
	return app_syscall1(syscall_prc_sleep, ms);
}

unsigned int * sdk_prc_getCanvas(){
	return (unsigned int *)app_syscall0(syscall_prc_getCurrentProcessScreenInfo);
}