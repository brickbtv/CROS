#include "app_disasm.h"
#include "sdk/os/process.h"
#include "sdk/scr/screen.h"
#include "sdk/scr/ScreenClass.h"
#include "sdk/os/debug.h"
#include "sdk/kyb/keyboard.h"
#include "utils/gui/GuiClass.h"
#include "stdlib/stdio_shared.h"
#include "stdlib/stdlib_shared.h"
#include "stdlib/string_shared.h"
#include "stdlib/containers/list.h"
#include "stdlib/extern/strtol/strtol.h"
#include "decoder/asm_decoder.h"

typedef struct APP_DISASM{
	ScreenClass * screen;
	GuiClass * gui;
	bool insPress;
	unsigned char * addr;
	unsigned char * next_addr;
	list_t * sym_table;
}APP_DISASM;

unsigned char * show_asm(ScreenClass * screen, unsigned char * addr, list_t * sym_table);

void msgHandlerDisasm(int type, int reason, int value, void * userdata){
	APP_DISASM * app = (APP_DISASM *)userdata;
	
	switch (type){
		case SDK_PRC_MESSAGE_KYB: 	
			if (value == KEY_INSERT){
				if (reason == KEY_STATE_KEYPRESSED)
					app->insPress = 1;
				else if (reason == KEY_STATE_KEYRELEASED)
					app->insPress = 0;
				
				break;
			}
			
			if (reason == KEY_STATE_KEYTYPED){
				if (app->insPress == true){
					if (value == 'x'){
						sdk_prc_die();
					}
					break;
				} 
				if (value == KEY_DOWN){
					app->next_addr = show_asm(app->screen, app->next_addr, app->sym_table);
				}
				
				if (value == KEY_UP){
					app->next_addr = show_asm(app->screen, app->next_addr - 8, app->sym_table);
				}
			}
	}
}

unsigned char * get_addr_from_args(const char * path){
	if (strncmp(path, "0x", 2) == 0){
		long addr = strtol(path, 0, 0);
		return (unsigned char *)addr;
	} 
}

unsigned char * show_asm(ScreenClass * screen, unsigned char * addr, list_t * sym_table){
	screen->clearArea(screen, CANVAS_COLOR_BLACK, 10, 1, screen->_canvas->res_hor - 10, screen->_canvas->res_ver - 2);
	
	unsigned char * addr_v = addr;
	
	screen->_canvas->cur_x = 0;
	screen->_canvas->cur_y = 0;
	
	unsigned char * second_instr;
	
	for (int i = 0; i < 23; i++){
		/*addr_v = */decode_instruction(screen->_canvas, (unsigned char *)addr_v, sym_table);
		addr_v += 4;
		if (i == 0)
			second_instr = addr_v;
	}
	
	return second_instr;
}

void app_disasm(const char * args){
	APP_DISASM app;
	
	Canvas * canvas = sdk_prc_getCanvas();
	app.screen = malloc(sizeof(ScreenClass));
	app.screen = ScreenClass_ctor(app.screen, canvas);
	
	app.gui= malloc(sizeof(GuiClass));
	app.gui = GuiClass_ctor(app.gui, canvas);
	
	unsigned char * addr = get_addr_from_args(args);
	sdk_debug_logf("%s, %d", args, addr);
	
	char title[256];	
	sprintf(title, "DisASM: %s (%d)", args, (int)addr);
	app.screen->clearScreen(app.screen, CANVAS_COLOR_BLACK);
	app.gui->draw_header(app.gui, title);
	app.gui->draw_bottom(app.gui, " UP/DOWN - scroll\tINS^x - exit");
	
	app.sym_table = init_symbol_table();
	
	app.next_addr = show_asm(app.screen, addr, app.sym_table);
	
	while (1){
		while (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHandlerDisasm, &app);
		}
		sdk_prc_sleep_until_new_messages();
	}
}