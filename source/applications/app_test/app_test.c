#include "app_test.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"

void app_test(void){
	Canvas * cv = sdk_prc_getCanvas();
	sdk_scr_clearScreen(cv, 0x4);
	sdk_scr_printfXY(cv, 10, 5, "TEstsetset");
	sdk_prc_sleep(1000);
	sdk_debug_log("DUMPed ELF");
	sdk_debug_log("Twice");
	int a = 10;
	a++;
	sdk_debug_log("Trice");
	sdk_debug_logf("MATH: %d", a);
	sdk_prc_die();
}

