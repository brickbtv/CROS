#include "app_test.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/nic/network.h"
#include "sdk/kyb/keyboard.h"

#include "filesystem/filesystem.h"

int mount_drive_and_make_fs_if_needed(Canvas * canvas){
	int res = fs_mount_drive(0);
	if (res != FS_OK){
		if (res == FS_NO_FILESYSTEM){
			sdk_scr_printf(canvas, "No file system found.\nMarking drive 0. It takes 1-2 minutes.\n");
			int res_mkfs = fs_make_filesystem();
			if (res_mkfs != FS_OK){
				sdk_scr_printf(canvas, "Failed to make filesytem. Abort.\n");
				return 1;
			} else {
				sdk_scr_printf(canvas, "Done.\n");
			}
		} else {
			sdk_scr_printf(canvas, "Failed to mount drive. It's mounted? Abort.\n");
			return 2;
		}
	} 
	
	return 0;
}

void app_test(void){
	Canvas * canvas = (Canvas*)sdk_prc_getCanvas();
	sdk_scr_clearScreen(canvas, SCR_COLOR_BLACK);
	
	int res = mount_drive_and_make_fs_if_needed(canvas);
	if (res){
		return;
	}

	res = fs_mkdir("CROS");
	if (res != FS_OK && res != FS_EXIST){
		sdk_scr_printf(canvas, "Failed to create directory.\n");
	} else if (res == FS_OK || res == FS_EXIST){
		FILE * file = fs_open_file("CROS/ABC.TXT", 'w');
		if (file){
			int res_write = fs_write_file(file, "EVERYBODY JUMPS");
			if (res_write != FS_OK){
				sdk_scr_printf(canvas, "Failed to write file\n");
			} else {
				fs_close_file(file);
				FILE * file2 = fs_open_file("CROS/ABC.TXT", 'r');
				char buf[1024];
				unsigned int rb;
				int rr = fs_read_file(file2, buf, 1024, &rb);
				if (rr != FS_OK){
					sdk_scr_printf(canvas, "Failed to read from file.\n");
				}
				fs_close_file(file2);
				sdk_scr_printf(canvas, "Readed from file: %s BR: %d\n", buf, rb);
			}
		} else {
			sdk_scr_printf(canvas, "Failed to open file\n");
		}
	}
	
	
	while(1){
		
	}
}