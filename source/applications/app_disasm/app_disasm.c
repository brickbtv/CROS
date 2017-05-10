#include "app_disasm.h"
#include "sdk/os/process.h"
#include "sdk/scr/screen.h"
#include "sdk/scr/ScreenClass.h"
#include "sdk/os/debug.h"
#include "utils/gui/GuiClass.h"
#include "stdlib/stdio_shared.h"
#include "stdlib/stdlib_shared.h"
#include "stdlib/string_shared.h"
#include "stdlib/extern/strtol/strtol.h"
#include "decoder/asm_decoder.h"

typedef struct APP_DISASM{
	ScreenClass * screen;
	GuiClass * gui;
}APP_DISASM;

void msgHandlerDisasm(int type, int reason, int value, void * userdata){

}

unsigned char * get_addr_from_args(const char * path){
	if (strncmp(path, "0x", 2) == 0){
		long addr = strtol(path, 0, 0);
		return (unsigned char *)addr;
	} 
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
	sprintf(title, "DisASM: %s (%d)", args, addr);
	app.screen->clearScreen(app.screen, SCR_COLOR_BLACK);
	app.gui->draw_header(app.gui, title);
	app.gui->draw_bottom(app.gui, "INS^x - exit");
	
	init_symbol_table();
	
	unsigned char * addr_v = addr;
	
	canvas->cur_x = 0;
	canvas->cur_y = 0;
	
	for (int i = 0; i < 23; i++){
		addr_v = decode_instruction(canvas, (unsigned char *)addr_v);
	}
	
	while (1){
		while (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHandlerDisasm, &app);
		}
		sdk_prc_sleep_until_new_messages();
	}
}