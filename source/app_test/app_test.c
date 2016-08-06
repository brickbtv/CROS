#include "app_test.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/nic/network.h"
#include "sdk/kyb/keyboard.h"

#include "app_texteditor/app_texteditor.h"

#include "filesystem/filesystem.h"

#include "sdk/os/process.h"

#include "app_texteditor/app_texteditor.h"

int run(){
	sdk_debug_log("IT's MY TRIUMPH");
	return 0;
}

void app_test(void){
	mount_drive_and_mkfs_if_needed(0);
	
	sdk_prc_create_process(app_texteditor);
	
	/*app_texteditor("/CONFIG/MAIL.C");

	Canvas * canvas = (Canvas*)sdk_prc_getCanvas();
	sdk_scr_clearScreen(canvas, SCR_COLOR_BLACK);
	
	short ram[1024];

	short * ss = (short *) sdk_debug_log;
	sdk_scr_printf(canvas, "sdk_debug_log %x %x\n", ss, *ss);
	
	short * prog = (short *) run;
	for (int i = 0; i < 20; i++){
		sdk_scr_printf(canvas, "%d %x %x\n", i, prog, *prog);
		ram[i] = *prog;
		prog++;
	}
	
	short fs = ram[0];
	sdk_scr_printf(canvas, "func_start %x\n", fs & 0x0000FFFF);
	
	*/
	
	while(1){
		
	}
}