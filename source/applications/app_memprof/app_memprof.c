#include "app_memprof.h"
#include "sdk/os/process.h"
#include "sdk/scr/screen.h"
#include "sdk/scr/ScreenClass.h"
#include "sdk/os/debug.h"
#include "sdk/kyb/keyboard.h"
#include "utils/gui/GuiClass.h"
#include "utils/timers_and_clocks/timers.h"
#include "stdlib/stdio_shared.h"
#include "stdlib/stdlib_shared.h"
#include "stdlib/string_shared.h"
#include "stdlib/extern/tlsf/tlsf.h"

typedef struct APP_MEMPROF{
	Canvas * canvas;
	ScreenClass * screen;
	GuiClass * gui;
	int insPress;	
	unsigned int timerId;
} APP_MEMPROF;

void msgHandlerMemProf(int type, int reason, int value, void * userdata){
	APP_MEMPROF * app = (APP_MEMPROF *)userdata;
	
	switch (type){
		case SDK_PRC_MESSAGE_CLK:
			timers_handleMessage(type, reason, value, userdata);
			break;
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

typedef struct MEMPROF_STAT{
	int used;
	int free;
} MEMPROF_STAT;

void pool_walker(void* ptr, size_t size, int used, void* userdata){
	MEMPROF_STAT * app = (MEMPROF_STAT *)userdata;
	if (used)
		app->used += size;
	else 
		app->free += size;
}

void updateMemMap(unsigned int timerId, void * userdata){
	APP_MEMPROF * app = (APP_MEMPROF *)userdata;
	if (app->timerId != timerId)
		return;
	app->screen->getCanvas(app->screen)->cur_x = 0;
	app->screen->getCanvas(app->screen)->cur_y = 1;
	ProcessDummy * prc;
	
	list_t * processes_list = sdk_prc_get_scheduler_list();
	list_node_t * node = processes_list->head;
	while (node){
		prc = (ProcessDummy *) node->val;
		
		app->screen->printf(app->screen, "%d - %s\n", prc->pid, prc->name);
		
		MEMPROF_STAT memstat;
		memset(&memstat, 0, sizeof(MEMPROF_STAT));
		
		pool_t pool = tlsf_get_pool(prc->heap);
		tlsf_walk_pool(pool, pool_walker, &memstat);
		
		int total = memstat.free + memstat.used;
		float prc_used = (float)memstat.used / (float)total * 100.0f;
		float prc_free = (float)memstat.free / (float)total * 100.0f;
		
		app->screen->printf(app->screen, "\ttotal: %d, free: %d (%.2f%), used: %d (%.2f%)\n",
		total,
		memstat.free,
		prc_free,
		memstat.used,
		prc_used);
		
		node = node->next;
	}	
}

void app_memprof(void){
	APP_MEMPROF * app = calloc(sizeof(APP_MEMPROF));
	
	app->canvas = sdk_prc_getCanvas();
	app->screen = malloc(sizeof(ScreenClass));
	app->screen = ScreenClass_ctor(app->screen, app->canvas);
	
	app->gui = malloc(sizeof(GuiClass));
	app->gui = GuiClass_ctor(app->gui, app->canvas);
	
	char * title = "MemProfiler";
	app->screen->clearScreen(app->screen, CANVAS_COLOR_BLACK);
	app->gui->draw_header(app->gui, title);
	app->gui->draw_bottom(app->gui, " INS^x - exit");
	
	app->timerId = timers_add_timer(1000, updateMemMap);
		
	while (1){
		if (sdk_prc_is_focused()){
			while (sdk_prc_haveNewMessage()){
				sdk_prc_handleMessage(msgHandlerMemProf, app);
			}
		}
		sdk_prc_sleep_until_new_messages();
	}
	
	sdk_prc_die();
}