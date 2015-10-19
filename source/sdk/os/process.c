#include "process.h"
#include "sdk/syscall_def.h"
#include "process/process.h"

unsigned int sdk_prc_sleep(unsigned int ms){
	return app_syscall1(syscall_prc_sleep, ms);
}

unsigned int * sdk_prc_getCanvas(){
	return (unsigned int *)app_syscall0(syscall_prc_getCurrentProcessScreenInfo);
}

bool sdk_prc_haveNewMessage(void){
	return (bool)app_syscall0(syscall_prc_haveNewMessages);
}

void sdk_prc_handleMessage(F_PRC_MSGCBACK cback){
	PrcMessage * msg = (PrcMessage *)app_syscall0(syscall_prc_getNextMessage);
	
	cback(msg->type, msg->reason, msg->value);
}