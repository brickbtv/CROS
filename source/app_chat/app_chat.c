#include "app_chat.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"

void app_chat(void){
	Canvas * canvas = (unsigned int*)sdk_prc_getCanvas();
	sdk_scr_printfXY(canvas, 0, 0, "test new app in user mode!\n");
	
	while (1){
		sdk_scr_printf(canvas, "test\n");
		sdk_prc_sleep(500);
	}
}