#include "app_test.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"

void app_test(void){	
	sdk_debug_log("DUMPed ELF");
	sdk_prc_die();
}