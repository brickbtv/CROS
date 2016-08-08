#include "process.h"
#include "sdk/syscall_def.h"
#include "process/process.h"

#include <details/memdetails.h>
#include "debug.h"

void sdk_prc_create_process(unsigned int entry_point, char * arg_line){
	app_syscall2(syscall_prc_create_process, (unsigned int)entry_point, (unsigned int)&arg_line[0]);
}

unsigned int sdk_prc_sleep(unsigned int ms){
	return app_syscall1(syscall_prc_sleep, ms);
}

unsigned int * sdk_prc_getCanvas(){
	return (unsigned int *)app_syscall0(syscall_prc_getCurrentProcessScreenInfo);
}

void * sdk_prc_getHeapPointer(){
	return (void*)app_syscall0(syscall_prc_getCurrentProcessHeapPointer);
}

bool sdk_prc_haveNewMessage(void){
	return (bool)app_syscall0(syscall_prc_haveNewMessages);
}

void sdk_prc_handleMessage(F_PRC_MSGCBACK cback){
	//PrcMessage * msg = (PrcMessage *)app_syscall0(syscall_prc_getNextMessage);
	unsigned int msg = app_syscall0(syscall_prc_getNextMessage);//msg->type, rs = msg->reason, vl = msg->value;

	int type = (msg   & 0xFF000000) >> 24; 
	int reason = (msg & 0x00FF0000) >> 16;
	int value = msg  & 0x0000FFFF;
			
	cback(type, reason, value);
}

void sdk_prc_lock(void){
	app_syscall0(syscall_prc_lock);
}

void sdk_prc_unlock(void){
	app_syscall0(syscall_prc_unlock);
}

bool sdk_prc_is_focused(void){
	return app_syscall0(syscall_prc_is_focused);
}

void sdk_prc_die(void){
	app_syscall0(syscall_prc_die);
	while(1){}
	//sdk_prc_sleep(10000);
}