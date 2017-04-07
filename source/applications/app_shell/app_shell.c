#include "app_shell.h"

#include <sdk/scr/screen.h>
#include <sdk/scr/ScreenClass.h>
#include <sdk/os/process.h>
#include <sdk/os/debug.h>
#include <sdk/kyb/keyboard.h>
#include <sdk/clk/clock.h>

#include <utils/filesystem/filesystem.h>
#include "commands/commands.h"

#include <string_shared.h>
#include <utils/timers_and_clocks/timers.h>

#include "stdlib/details/memdetails.h"


static char input[128];
static int symb = 0;
static Canvas * canvas;
static ScreenClass * shell_screen;
static char current_path[256];
static int path_lenght = 0;
static bool blink = true;

void blinkCBack(unsigned int tn){
	blink = ! blink;
	
	shell_screen->printfXY(shell_screen, canvas->cur_x, canvas->cur_y, blink?"_":" ");
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
					shell_screen->printfXY(shell_screen, canvas->cur_x, canvas->cur_y, " ");
					canvas->cur_x --;
					if (--symb <= 0)
						symb = 0;
					input[symb] = 0;
				} else if (value == KEY_RETURN){
					shell_screen->printfXY(shell_screen, canvas->cur_x, canvas->cur_y, " ");
					shell_screen->printf(shell_screen, "\n");				
					
					// changig color for commands output!
					shell_screen->setTextColor(shell_screen, SCR_COLOR_WHITE);
					manage_command(shell_screen, current_path, input);
					shell_screen->setTextColor(shell_screen, SCR_COLOR_GREEN);
					
					while (!sdk_prc_is_focused()){
						sdk_prc_sleep(1000);
					}
					
					memset(input, 0, 128 * sizeof(char));
					symb = 0;
				} else {
					if (symb >= shell_screen->getScreenWidth(shell_screen) - strlen(current_path) - 1)	// command line can't be longer
						break;
										
					input[symb++] = value;
				}
				shell_screen->printfXY(shell_screen, path_lenght, canvas->cur_y, "%s>%s", current_path, input);
			}
			break;
	}
}

int mount_drive_and_mkfs_if_needed(){
	int res = fs_mount_drive(0);
	if (res != FS_OK){
		if (res == FS_NO_FILESYSTEM){
			shell_screen->printf(shell_screen, "No file system found.\nMarking drive 0. It takes 1-2 minutes.\n");
			int res_mkfs = fs_make_filesystem();
			if (res_mkfs != FS_OK){
				shell_screen->printf(shell_screen, "Failed to make filesytem. Abort.\n");
				return 1;
			} else {
				shell_screen->printf(shell_screen, "Done.\n");
			}
		} else {
			shell_screen->printf(shell_screen, "Failed to mount drive. It's mounted? Abort.\n");
			return 2;
		}
	} 
	
	return 0;
}

void app_shell(void){
	canvas = (Canvas*)sdk_prc_getCanvas();
	shell_screen = malloc(sizeof(ScreenClass));
	shell_screen = ScreenClass_ctor(shell_screen, canvas);
	
	timers_add_timer(0, 500, blinkCBack);
	
	shell_screen->clearScreen(shell_screen, SCR_COLOR_BLACK);
	
	if (mount_drive_and_mkfs_if_needed() == 2)
		while(1){};	
	
	fs_getcwd(current_path, 256);
	shell_screen->printf(shell_screen, "CR Shell. Version 1.0.\nWelcome!\nType 'help' for commands list\n");
	shell_screen->printf(shell_screen, "%s>", current_path);

	while(1){
		if (sdk_prc_is_focused()){		
			if (sdk_prc_haveNewMessage()){
				sdk_prc_handleMessage(msgHandlerShell);
			}
		}
	}
}