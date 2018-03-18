#include "network.h"

#include "sdk/syscall_def.h"

#include <string_shared.h>
#include <stdarg_shared.h>
#include <stdio_shared.h>

int sdk_nic_getAddress(void){
	return app_syscall0(syscall_nic_address);
}

void sdk_nic_send(unsigned int addr, const char * msg, unsigned int size){
	app_syscall3(syscall_nic_send, addr, (unsigned int)&msg[0], size);
}

void sdk_nic_sendf(unsigned int addr, const char * fmt, ...){
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	app_syscall3(syscall_nic_send, addr, (unsigned int)&buf[0], strlen(buf) + 1);
}

int sdk_nic_recv(char * msg, int max_size, int * addr){
	return app_syscall3(syscall_nic_recv, (unsigned int)&msg[0], max_size, (unsigned int)addr);
}