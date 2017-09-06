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
	int work;
} APP_MEMPROF;

void memprof_exit(APP_MEMPROF * app){
	app->work = 0;
	timers_del_timer(app->timerId);
	free(app->gui);
	free(app->screen);
		
	free(app);
	
	sdk_prc_die();
}

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
						memprof_exit(app);
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

float bytes_to_kbytes(unsigned int bytes){
	return ((float)bytes)/1024.0f;
}

void updateMemMap(unsigned int timerId, void * userdata){
	APP_MEMPROF * app = (APP_MEMPROF *)userdata;
	if (app->timerId != timerId)
		return;
		
	ScreenClass * screen = app->screen;
	screen->getCanvas(screen)->cur_x = 0;
	screen->getCanvas(screen)->cur_y = 1;
	screen->clearArea(screen,
							CANVAS_COLOR_BLACK,
							0, 1, 
							screen->getCanvas(screen)->res_hor,
							screen->getCanvas(screen)->res_ver - 2);
	ProcessDummy * prc;
	
	//screen->printf(screen, "TOTAL RAM: %.2f KB\n", bytes_to_kbytes(sdk_prc_get_total_memory()));
	screen->setBackColor(screen, CANVAS_COLOR_WHITE);
	screen->setTextColor(screen, CANVAS_COLOR_BLACK);
	screen->printf(screen, "%-4s %-20s %-10s %-10s %-10s %-10s %-10s\n", 
							"PID", "Name", "Total, KB", 
							"Free, KB", "Free, %", "Used, KB", "Used, %");
	screen->setBackColor(screen, CANVAS_COLOR_BLACK);
	screen->setTextColor(screen, CANVAS_COLOR_WHITE);
	
	list_t * processes_list = sdk_prc_get_scheduler_list();
	list_node_t * node = processes_list->head;
	unsigned int total_used = 0;
	while (node){
		prc = (ProcessDummy *) node->val;
		
		//screen->printf(screen, "%d - %s\n", prc->pid, prc->name);
		
		MEMPROF_STAT memstat;
		memset(&memstat, 0, sizeof(MEMPROF_STAT));
		
		pool_t pool = tlsf_get_pool(prc->heap);
		tlsf_walk_pool(pool, pool_walker, &memstat);
		
		int total = memstat.free + memstat.used;
		float prc_used = (float)memstat.used / (float)total * 100.0f;
		float prc_free = (float)memstat.free / (float)total * 100.0f;
		
		#define MEMPROF_MAXNAME 20
		
		char fixname[MEMPROF_MAXNAME];
		if (strlen(prc->name) > MEMPROF_MAXNAME){
			strncpy(fixname, prc->name, MEMPROF_MAXNAME);
			fixname[MEMPROF_MAXNAME - 1] = 0;
		} else {
			strcpy(fixname, prc->name);
			fixname[strlen(prc->name)] = 0;
		}
		
		//screen->printf(screen, "\ttotal: %.2f KB, free: %.2f (%.2f%), used: %.2f KB (%.2f%)\n",
		screen->printf(screen, "%-4d %-20s %-10.2f %-10.2f %-10.2f %-10.2f %-10.2f\n",
							prc->pid, fixname,
							bytes_to_kbytes(total),
							bytes_to_kbytes(memstat.free),
							prc_free,
							bytes_to_kbytes(memstat.used),
							prc_used);
		
		total_used += memstat.used;
		
		node = node->next;
	}	
	
	unsigned int total = sdk_prc_get_total_memory();
	
	float prc_used = (float)total_used / (float)total * 100.0f;
	float prc_free = (float)(total - total_used) / (float)total * 100.0f;
	
	screen->setBackColor(screen, CANVAS_COLOR_WHITE);
	screen->setTextColor(screen, CANVAS_COLOR_BLACK);
	screen->printf(screen, "%-4s %-20s %-10.2f %-10.2f %-10.2f %-10.2f %-10.2f\n",
							"", "TOTAL:",
							bytes_to_kbytes(sdk_prc_get_total_memory()),
							bytes_to_kbytes(total - total_used),
							prc_free,
							bytes_to_kbytes(total_used),
							prc_used);
	screen->setBackColor(screen, CANVAS_COLOR_BLACK);
	screen->setTextColor(screen, CANVAS_COLOR_WHITE);
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
	
	updateMemMap(app->timerId, app);
	
	app->work = 1;
		
	while (app->work){
		if (sdk_prc_is_focused()){
			while (sdk_prc_haveNewMessage()){
				sdk_prc_handleMessage(msgHandlerMemProf, app);
			}
		}
		sdk_prc_sleep_until_new_messages();
	}
	
	memprof_exit(app);
}