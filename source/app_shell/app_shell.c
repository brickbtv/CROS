#include "app_shell.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/kyb/keyboard.h"
#include "sdk/clk/clock.h"

#include "filesystem/filesystem.h"

#include <string_shared.h>

//#include "filesystem/fatfs/src/ff.h"

static char input[1024];
static int symb = 0;
static Canvas * canvas;
static char current_path[256];
static int path_lenght = 0;
static bool blink = true;

void manage_command(){
	if (strcmp(input, "help") == 0){
		sdk_scr_printf(canvas, "wow! such help! much text!\n"
		" 'ls' - show files and folders in current directory\n"
		" 'mkdir NAME' - make new directory\n"
		" 'mkfile NAME' - make new file\n"
		" 'cd NAME' - change directory\n"
		" 'rm NAME' - remove file or directory\n");
	} else if (strcmp(input, "ls") == 0){
		FILEINFO fno;
		int i;
		char *fn;
		FOLDER* dir = fs_opendir(current_path);                       	/* Open the directory */
		if (dir) {
			i = strlen(current_path);
			for (;;) {
				int res = fs_readdir(dir, &fno);                   		/* Read a directory item */
				if (res != FS_OK || fno.fname[0] == 0) break;  			/* Break on error or end of dir */
				
				sdk_scr_printf(canvas, "%c %s\n", fno.ftype, fno.fname);
			}
			fs_closedir(dir);
		}

	} else if (strncmp(input, "mkdir ", strlen("mkdir ")) == 0){
		fs_mkdir(&input[strlen("mkdir ")]);
	} else if (strncmp(input, "mkfile ", strlen("mkfile ")) == 0){
		FILE * file = fs_open_file(&input[strlen("mkfile ")], 'w');
		if (file){
			fs_close_file(file);
		} else {
			sdk_scr_printf(canvas, "Failed to create file.\n");
		}
	} else if (strncmp(input, "cd ", strlen("cd ")) == 0){
		if (strcmp(&input[strlen("cd ")], "..") != 0) {
			current_path[strlen(current_path)] = '/';
			strcpy(&current_path[strlen(current_path)], &input[strlen("cd ")]);
		} else {
			// up to parent folder
			for (int i = strlen(current_path); i >= 0; i--){
				if (current_path[i] == '/'){
					current_path[i] = 0;
					return;
				}
			}
		}
		
	} else if (strncmp(input, "rm ", strlen("rm ")) == 0){
		fs_unlink(&input[strlen("rm ")]);
	}
}

void msgHandlerShell(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_KYB: 
			if (reason == KEY_STATE_KEYTYPED){
				if (value == 0x01){
					// clear cursor before backspace
					sdk_scr_printfXY(canvas, canvas->cur_x, canvas->cur_y, " ");
					canvas->cur_x --;
					if (--symb <= 0){
						symb = 0;
					}
					input[symb] = 0;
				} else if (value == 0x02){
					// clear cursor before enter
					sdk_scr_printfXY(canvas, canvas->cur_x, canvas->cur_y, " ");
					canvas->cur_x --;
					sdk_scr_printf(canvas, "\n");
					
					manage_command();
					
					memset(input, 0, 1024 * sizeof(char));
					symb = 0;
				} else {
					if (symb >= 80)	// command line can't be longer
						break;
						
					input[symb++] = value;
				}
				sdk_scr_printfXY(canvas, path_lenght, canvas->cur_y, "%s>%s", current_path, input);
			}
			break;
	}
}

int mount_drive_and_mkfs_if_needed(Canvas * canvas){
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

void app_shell(void){
	canvas = (Canvas*)sdk_prc_getCanvas();
	sdk_scr_clearScreen(canvas, SCR_COLOR_BLACK);
	
	mount_drive_and_mkfs_if_needed(canvas);
	
	sdk_scr_printf(canvas, "CR Shell. Version 1.0.\nWelcome!\nType 'help' for commands list\n");
	sdk_scr_printf(canvas, "%s>", current_path);
	// cursor blinker. 
	//
	// YES, RUI, I STILL COMPELLED TO IMPLEMENT BLINKING SINGLY.
	// 
	// every 0.5 sec
	sdk_clk_setCountdownTimer(5, 500, true);
	bool prev_blink = false;
	while(1){
		int time = sdk_clk_timeSinceBoot();
		if (time % 1000 < 500){
			blink = true;
		} else {
			blink = false;
		}
		if (prev_blink != blink){
			if (blink)
				sdk_scr_printfXY(canvas, canvas->cur_x, canvas->cur_y, "_");
			else 
				sdk_scr_printfXY(canvas, canvas->cur_x, canvas->cur_y, " ");
			canvas->cur_x--;
			prev_blink = blink;
		}
	
		if (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHandlerShell);
		}
		
		//sdk_prc_sleep(10);
	}
}