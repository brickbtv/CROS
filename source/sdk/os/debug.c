#include "debug.h"
#include "sdk/syscall_def.h"
#include <string_shared.h>

void sdk_debug_log(const char * msg){
	app_syscall2(syscall_nic_debug, (int)&msg[0], strlen(msg) + 1);
}