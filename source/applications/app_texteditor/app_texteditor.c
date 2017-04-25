#include "app_texteditor.h"

#include <sdk/os/process.h>
#include <sdk/os/debug.h>
#include <sdk/kyb/keyboard.h>
#include <sdk/clk/clock.h>

#include <utils/filesystem/filesystem.h>
#include <utils/gui/GuiClass.h>
#include <utils/timers_and_clocks/timers.h>
#include <sdk/scr/screen.h>
#include <sdk/scr/ScreenClass.h>

#include <string_shared.h>
#include <containers/list.h>

#include <utils/gui/gui.h>
#include <utils/gui/editbox/editbox.h>

#include <preprocessor/preprocessor.h>

typedef struct TEXTEDITOR{
	ScreenClass * screen;
	GuiClass * gui;
	EditBoxClass * editbox;

	list_t* text_lines;

	char path[256];

	int insPress;
	int exit;
	bool blink;
	int timer_id;
} TEXTEDITOR;

void close_app(TEXTEDITOR * te);

void msgHandlerTexteditor(int type, int reason, int value, void * userdata){
	TEXTEDITOR * te = (TEXTEDITOR *)userdata;
	te->editbox->handle_message(te->editbox, type, reason, value);
	switch (type){
		case SDK_PRC_MESSAGE_CLK:
			timers_handleMessage(type, reason, value, userdata);
			break;
		case SDK_PRC_MESSAGE_KYB: 	
			// CTRL rplsmnt by INSERT
			if (value == KEY_INSERT){
				if (reason == KEY_STATE_KEYPRESSED)
					te->insPress = 1;
				else if (reason == KEY_STATE_KEYRELEASED)
					te->insPress = 0;
				
				break;
			}
			
			if (reason == KEY_STATE_KEYTYPED){
				// letters
				if (value >= 0x20 && value <= 0x7E){
					if (te->insPress  == true && value == 's'){
						sdk_debug_log("saving");
						
						FILE * file = fs_open_file(te->path, 'w');
						
						list_iterator_t * it = list_iterator_new(te->text_lines, LIST_HEAD);
						list_node_t *node;
						char end_line[2] = {'\n', 0};
						while ((node = list_iterator_next(it))){
							fs_write_file(file, node->val);
							if (node->next) // except last line
								fs_write_file(file, end_line);
						}
						fs_close_file(file);
						list_iterator_destroy(it);
						break;
					}
					
					if (te->insPress  == true && value == 'x'){
						sdk_debug_log("exit");
						//exit = 1;
						close_app(te);
					}
				}
			}
			break;
	}
}

void teBlinkCBack(unsigned int tn, void * userdata){
	TEXTEDITOR * te = (TEXTEDITOR *)userdata;
	if (tn == te->timer_id){
		te->blink = !te->blink;
		te->editbox->set_blink(te->editbox, te->blink);
	}
}

void redraw_text_area(TEXTEDITOR * te, int start_line){
	te->screen->clearScreen(te->screen, SCR_COLOR_WHITE);
	te->gui->draw_header(te->gui, te->path);
	te->gui->draw_bottom(te->gui, " INS^s - save    INS^x - quit");

	te->editbox->redraw(te->editbox, start_line);
}

void close_app(TEXTEDITOR * te){
	free(te->editbox);
	free(te->gui);
	free(te->screen);
	
	timers_del_timer(te->timer_id);
	
	close_preprocessed_file(te->text_lines);
	sdk_prc_die();
}

void app_texteditor(const char* p){	
	Canvas * cv = (Canvas*)sdk_prc_getCanvas();
	
	TEXTEDITOR * te = calloc(sizeof(TEXTEDITOR));
		
	te->exit = 0;
	te->insPress = 0;
	
	te->gui = malloc(sizeof(GuiClass));
	te->gui = GuiClass_ctor(te->gui, cv);
	
	te->screen = malloc(sizeof(ScreenClass));
	te->screen = ScreenClass_ctor(te->screen, cv);
		
	te->screen->clearScreen(te->screen, SCR_COLOR_WHITE);
	
	te->editbox = malloc(sizeof(EditBoxClass));
	te->editbox = EditBoxClass_ctor(te->editbox, 
								te->screen,
								0, 
								1,
								te->screen->getScreenHeight(te->screen) - 2, 
								te->screen->getScreenWidth(te->screen));
	
	memset(te->path, 0, sizeof(char) * 256);
	strcpy(te->path, p);
	
	te->text_lines = preprocess_file(p, te->screen->getScreenWidth(te->screen));
	te->editbox->set_list(te->editbox, te->text_lines);		
	
	redraw_text_area(te, 0);
	
	te->timer_id = timers_add_timer(500, teBlinkCBack);
	
	while(te->exit == 0){		
	
		while (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHandlerTexteditor, te);
		}
		sdk_prc_sleep_until_new_messages();
	}
	
	close_app(te);
}