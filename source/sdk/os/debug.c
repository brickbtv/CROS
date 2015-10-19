#include "debug.h"
#include "sdk/syscall_def.h"
#include <stdarg_shared.h>
#include <stdlib_shared.h>
#include <stdio_shared.h>
#include <string_shared.h>

void sdk_debug_log(const char * msg){
	app_syscall2(syscall_nic_debug, (int)&msg[0], strlen(msg) + 1);
}

void sdk_debug_logf(const char * fmt, ...){
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	app_syscall2(syscall_nic_debug, (int)&buf[0], strlen(buf) + 1);
}