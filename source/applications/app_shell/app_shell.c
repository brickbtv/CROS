#include "app_shell.h"

#include <sdk/scr/screen.h>
#include <sdk/os/process.h>
#include <sdk/os/debug.h>
#include <sdk/kyb/keyboard.h>
#include <sdk/clk/clock.h>

#include <utils/filesystem/filesystem.h>
#include "commands/commands.h"

#include <string_shared.h>
#include <utils/timers_and_clocks/timers.h>

static char input[128];
static int symb = 0;
static Canvas * canvas;
static char current_path[256];
static int path_lenght = 0;
static bool blink = true;

void blinkCBack(unsigned int tn){
	blink = ! blink;
	
	sdk_scr_printfXY(canvas, canvas->cur_x, canvas->cur_y, blink?"_":" ");
	canvas->cur_x--;
}

void msgHandlerShell(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_CLK:
			timers_handleMessage(type, reason, value);
			break;
		case SDK_PRC_MESSAGE_KYB: 
			if (reason == KEY_STATE_KEYTYPED){
				if (value == KEY_BACKSPACE){
					sdk_scr_printfXY(canvas, canvas->cur_x, canvas->cur_y, " ");
					canvas->cur_x --;
					if (--symb <= 0)
						symb = 0;
					input[symb] = 0;
				} else if (value == KEY_RETURN){
					sdk_scr_printfXY(canvas, canvas->cur_x, canvas->cur_y, " ");
					sdk_scr_printf(canvas, "\n");				
					
					// changig color for commands output!
					sdk_scr_setTextColor(canvas, SCR_COLOR_WHITE);
					manage_command(canvas, current_path, input);
					sdk_scr_setTextColor(canvas, SCR_COLOR_GREEN);
					
					while (!sdk_prc_is_focused()){
						sdk_prc_sleep(1000);
					}
					
					memset(input, 0, 128 * sizeof(char));
					symb = 0;
				} else {
					if (symb >= canvas->res_hor - strlen(current_path) - 1)	// command line can't be longer
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
	timers_add_timer(0, 500, blinkCBack);

	canvas = (Canvas*)sdk_prc_getCanvas();
	sdk_scr_clearScreen(canvas, SCR_COLOR_BLACK);
	
	if (mount_drive_and_mkfs_if_needed(canvas) == 2)
		while(1){};	
	
	fs_getcwd(current_path, 256);
	sdk_scr_printf(canvas, "CR Shell. Version 1.0.\nWelcome!\nType 'help' for commands list\n");
	sdk_scr_printf(canvas, "%s>", current_path);

	while(1){
		if (sdk_prc_is_focused()){		
			if (sdk_prc_haveNewMessage()){
				sdk_prc_handleMessage(msgHandlerShell);
			}
		}
	}
}