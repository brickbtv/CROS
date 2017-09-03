#include "app_memprof.h"
#include "sdk/os/process.h"
#include "sdk/scr/screen.h"
#include "sdk/scr/ScreenClass.h"
#include "sdk/os/debug.h"
#include "sdk/kyb/keyboard.h"
#include "utils/gui/GuiClass.h"
#include "stdlib/stdio_shared.h"
#include "stdlib/stdlib_shared.h"
#include "stdlib/string_shared.h"

typedef struct APP_MEMPROF{
	ScreenClass * screen;
	GuiClass * gui;
	int insPress;	
} APP_MEMPROF;

void msgHandlerMemProf(int type, int reason, int value, void * userdata){
	APP_MEMPROF * app = (APP_MEMPROF *)userdata;
	
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
					
				}
			}
	}
}

void app_memprof(const char * args){
	APP_MEMPROF app;
	
	Canvas * canvas = sdk_prc_getCanvas();
	app.screen = malloc(sizeof(ScreenClass));
	app.screen = ScreenClass_ctor(app.screen, canvas);
	
	app.gui= malloc(sizeof(GuiClass));
	app.gui = GuiClass_ctor(app.gui, canvas);
	
	char * title = "MemProfiler";
	app.screen->clearScreen(app.screen, CANVAS_COLOR_BLACK);
	app.gui->draw_header(app.gui, title);
	app.gui->draw_bottom(app.gui, " INS^x - exit");
	
	while (1){
		while (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHandlerMemProf, &app);
		}
		sdk_prc_sleep_until_new_messages();
	}
}