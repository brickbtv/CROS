#include "app_test.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/nic/network.h"
#include "sdk/kyb/keyboard.h"
//#include "sdk/dkc/disk_drive.h"
#include "filesystem/fatfs/src/ff.h"



void app_test(void){
	Canvas * canvas2 = (Canvas*)sdk_prc_getCanvas();
	sdk_scr_clearScreen(canvas2, SCR_COLOR_BLACK);
	
	FATFS fs;
	/*char drive_name[2];
	drive_name[0] = (char)0;
	drive_name[1] = 0;*/
	sdk_debug_logf("1");
	FRESULT res = f_mount(&fs, "0:", 0);
	sdk_debug_logf("2 %d", res);
	
	DIR dir;
	res = f_opendir(&dir, "0:none.fil");
	sdk_debug_logf("3 %d", res);
	if (res == FR_NO_FILESYSTEM){
		res = f_mkfs("0:", 0, 0);
		sdk_debug_logf("4 %d", res);
	}
	
	res = f_opendir(&dir, "0:none.fil");
	sdk_debug_logf("5 %d", res);
	
	FIL file;
	res = f_open(&file, "0:TEST.X", FA_CREATE_ALWAYS);
	sdk_debug_logf("6 %d", res);
	
	
	while(1){
	
	}
}