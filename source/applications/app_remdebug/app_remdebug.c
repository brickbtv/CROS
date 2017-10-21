#include "app_remdebug.h"

#include "utils/filesystem/filesystem.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/dkc/disk_drive.h"

#include "stdlib/details/memdetails.h"
#include "sdk/scr/ScreenClass.h"

#include "containers/list.h"

#include <string_shared.h>
#include <stdio_shared.h>
#include <stdlib_shared.h>

void app_remdebug(void){
	Canvas * cv = (Canvas*)sdk_prc_getCanvas();
	ScreenClass * screen = malloc(sizeof(ScreenClass));
	screen = ScreenClass_ctor(screen, cv);
	
	int alive = 1;
	
	char * flag_filename = "0:/DEBUG";
	
	while (alive){
		
		if (fs_exists(flag_filename) == FS_OK){
			FILE * file = fs_open_file(flag_filename, 'r');
			char filename[128];
			int rb;
			fs_read_file(file, filename, 128, &rb);
			sdk_debug_logf("DEBUG_MODE... File: %s", filename);
			fs_close_file(file);
			fs_remove(flag_filename);
		}
		sdk_prc_sleep(1000);
	}
	
	free(screen);
}