#include "app_test.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/nic/network.h"
#include "sdk/kyb/keyboard.h"
//#include "sdk/dkc/disk_drive.h"
#include "filesystem/fatfs/src/ff.h"

#include "filesystem/fatfs/src/diskio.h"

void app_test(void){
	Canvas * canvas2 = (Canvas*)sdk_prc_getCanvas();
//	sdk_scr_clearScreen(canvas2, SCR_COLOR_BLACK);
	
	FATFS fs;
	
	/*char buff[512];
	char wbuff[512];
	wbuff[0] = 'a';
	wbuff[1] = 'b';
	wbuff[2] = 'c';
	for (int i = 3; i < 512-23; i++){
		wbuff[i] = 'd';
	}
	
	for (int i = 512-23; i < 512; i++){
		wbuff[i] = 'a' + i;
	}
	
	for (int i = 0; i < 512; i++){
		buff[i] = 0;
	}
	
	disk_write(0, wbuff, 0, 1);
	disk_read(0, buff, 0, 1);
	
	
	char * a = buff;
	sdk_debug_logf("sec_1: %c", *a);
	for (int i = 0; i < 512; i++){
		//if (*a != 0){
			char b = buff[i];
			sdk_debug_logf("sec_0: %d, %c", i, *a);
		//}
		a++;
	}
	*/
	
	char drive_name[2];
	drive_name[0] = (char)0;
	drive_name[1] = 0;
	
	
	FRESULT res = f_mount(&fs, "", 1);
	if (res != FR_OK){
		sdk_debug_logf("f_mount: %d", res);
		res = f_mkfs("", 0, 0);
		if (res != FR_OK){
			sdk_debug_logf("f_mkfs: %d", res);
		} else {
			FIL fil;
			//res = f_open(&fil, "NOO.TXT", FA_CREATE_NEW);
			res = f_mount(&fs, "", 1);
			if (res != FR_OK){
				sdk_debug_logf("f_open: %d", res);
			} else {
				sdk_debug_logf("IMPOSSSIBLE");
			}
		}
	} else {
		FIL fil;
		res = f_open(&fil, "NOO.TXT", FA_WRITE);
		if (res != FR_OK){
			sdk_debug_logf("f_open: %d", res);
		} else {
			sdk_debug_logf("IMPOSSSIBLE");
			UINT bw; 
			f_write(&fil, "MAKE IT EASIER", 15, &bw);
			if (res != FR_OK){
				sdk_debug_logf("f_write: %d", res);
			} else {
				f_close(&fil);
				res = f_open(&fil, "NOO.TXT", FA_READ);
				char ge[1024];
				res = f_read(&fil, ge, 1024, &bw);
				sdk_debug_logf("Nuuuuuu! : %s", ge);
			}
		}
	
		sdk_debug_logf("IMPOSSSIBLE NUMBER TWO");
	}
	
	
	while(1){
		
	}
}