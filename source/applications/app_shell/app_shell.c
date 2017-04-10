#include "app_shell.h"

#include <sdk/scr/screen.h>
#include <sdk/scr/ScreenClass.h>
#include <sdk/os/process.h>
#include <sdk/os/debug.h>
#include <sdk/kyb/keyboard.h>
#include <sdk/clk/clock.h>

#include <utils/filesystem/filesystem.h>
#include "commands/commands.h"
#include "mkfs/mkfs.h"

#include <string_shared.h>
#include <utils/timers_and_clocks/timers.h>

#include "stdlib/details/memdetails.h"

typedef struct APP_SHELL{
	char input[128];
	int symb;
	ScreenClass * screen;
	char current_path[256];
	int path_lenght;
	bool blink;
}APP_SHELL;

static APP_SHELL shell;

void blinkCBack(unsigned int tn){
	shell.blink = ! shell.blink;
	
	shell.screen->printfXY(	shell.screen, 
							shell.screen->_canvas->cur_x, 
							shell.screen->_canvas->cur_y, 
							shell.blink?"_":" ");
	shell.screen->_canvas->cur_x--;
}

void shellBackspace(){
	shell.screen->printfXY(	shell.screen, 
							shell.screen->_canvas->cur_x, 
							shell.screen->_canvas->cur_y, 
							" ");
							
	shell.screen->_canvas->cur_x --;
	if (--shell.symb <= 0)
		shell.symb = 0;
	shell.input[shell.symb] = 0;
}

void shellExec(){
	shell.screen->printfXY(	shell.screen, 
							shell.screen->_canvas->cur_x, 
							shell.screen->_canvas->cur_y, 
							" ");
	shell.screen->printf(shell.screen, "\n");				
	
	shell.screen->setTextColor(shell.screen, SCR_COLOR_WHITE);
	manage_command(shell.screen, shell.current_path, shell.input);
	shell.screen->setTextColor(shell.screen, SCR_COLOR_GREEN);

	memset(shell.input, 0, 128 * sizeof(char));
	shell.symb = 0;
}

void shellAddSymbolToCommand(int value){
	int screen_width = shell.screen->getScreenWidth(shell.screen);
	
	// command line can't be longer
	if (shell.symb >= screen_width  - strlen(shell.current_path) - 1)	
		return;
						
	shell.input[shell.symb++] = value;
}

void msgHandlerShell(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_CLK:
			timers_handleMessage(type, reason, value);
			break;
		case SDK_PRC_MESSAGE_KYB: 
			if (reason == KEY_STATE_KEYTYPED){
				if (value == KEY_BACKSPACE){
					shellBackspace();
				} else if (value == KEY_RETURN){
					shellExec();
				} else {
					shellAddSymbolToCommand(value);
				}
				shell.screen->printfXY(	shell.screen, 
										shell.path_lenght, 
										shell.screen->_canvas->cur_y, 
										"%s>%s", 
										shell.current_path, 
										shell.input);				
			}
			break;
	}
}

void initShellApp(){
	shell.blink = false;
	shell.symb = 0;
	shell.path_lenght = 0;
	memset(shell.current_path, 0, 256);
	memset(shell.input, 0, 128);
	
	Canvas * canvas = (Canvas*)sdk_prc_getCanvas();
	shell.screen = malloc(sizeof(ScreenClass)); 
	shell.screen = ScreenClass_ctor(shell.screen, canvas);
	
	shell.screen->clearScreen(shell.screen, SCR_COLOR_BLACK);
}

#define APP_SHELL_HELLO	"CR Shell. Version 1.0.\n" \
						"Welcome!\n" \
						"Type \'help\' for commands list\n"

void printHelloMessage(){
	fs_getcwd(shell.current_path, 256);
	shell.screen->printf(shell.screen, APP_SHELL_HELLO);
	shell.screen->printf(shell.screen, "%s>", shell.current_path);
}

void app_shell(void){
	initShellApp()
		
	timers_add_timer(0, 500, blinkCBack);
		
	if (mount_drive_and_mkfs_if_needed(shell.screen) == 2)
		while(1){};	
		
	printHelloMessage();

	while(1){
		if (sdk_prc_is_focused()){		
			if (sdk_prc_haveNewMessage()){
				sdk_prc_handleMessage(msgHandlerShell);
			}
			sdk_prc_sleep(50);
		}
	}
}