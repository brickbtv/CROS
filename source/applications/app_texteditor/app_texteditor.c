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

ScreenClass * screen;
GuiClass * gui;
EditBoxClass * editbox;

list_t* text_lines;

char path[256];

int insPress;
int exit = 0;
bool blink = false;

void msgHandlerTexteditor(int type, int reason, int value){
	editbox->handle_message(editbox, type, reason, value);
	switch (type){
		case SDK_PRC_MESSAGE_CLK:
			timers_handleMessage(type, reason, value);
			break;
		case SDK_PRC_MESSAGE_KYB: 	
			// CTRL rplsmnt by INSERT
			if (value == KEY_INSERT){
				if (reason == KEY_STATE_KEYPRESSED)
					insPress = 1;
				else if (reason == KEY_STATE_KEYRELEASED)
					insPress = 0;
				
				break;
			}
			
			if (reason == KEY_STATE_KEYTYPED){
				// letters
				if (value >= 0x20 && value <= 0x7E){
					if (insPress  == true && value == 's'){
						sdk_debug_log("saving");
						
						FILE * file = fs_open_file(path, 'w');
						
						list_iterator_t * it = list_iterator_new(text_lines, LIST_HEAD);
						list_node_t *node;
						while ((node = list_iterator_next(it))){
							char end_line[2];
							end_line[0] = '\n';
							end_line[1] = 0;
							fs_write_file(file, node->val);
							fs_write_file(file, end_line);
						}
						fs_close_file(file);
						list_iterator_destroy(it);
						break;
					}
					
					if (insPress  == true && value == 'x'){
						sdk_debug_log("exit");
						exit = 1;
					}
				}
			}
			break;
	}
}

void teBlinkCBack(unsigned int tn){
	if (tn == 11){
		blink = !blink;
		editbox->set_blink(editbox, blink);
	}
}

void redraw_text_area(int start_line){
	screen->clearScreen(screen, SCR_COLOR_BLACK);
	gui->draw_header(gui, path);
	gui->draw_bottom(gui, " INS^s - save    INS^x - quit");

	editbox->redraw(editbox, start_line);
}

void app_texteditor(const char* p){	
	Canvas * cv = (Canvas*)sdk_prc_getCanvas();
		
	exit = 0;
	insPress = 0;
	
	gui = malloc(sizeof(GuiClass));
	gui = GuiClass_ctor(gui, cv);
	
	screen = malloc(sizeof(ScreenClass));
	screen = ScreenClass_ctor(screen, cv);
		
	screen->clearScreen(screen, SCR_COLOR_BLACK);
	
	editbox = malloc(sizeof(EditBoxClass));
	editbox = EditBoxClass_ctor(editbox, 
								screen,
								0, 
								1
								screen->getScreenHeight(screen) - 2, 
								screen->getScreenWidth(screen));
	
	memset(path, 0, sizeof(char) * 256);
	strcpy(path, p);
	
	text_lines = preprocess_file(p, screen->getScreenWidth(screen));
	editbox->set_list(editbox, text_lines);		
	
	redraw_text_area(0);
	
	timers_add_timer(11, 500, teBlinkCBack);
	
	while(exit == 0){		
		//draw_blink();
	
		if (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHandlerTexteditor);
		}
		sdk_prc_sleep(100);
	}
	
	free(gui);
	free(screen);
	
	timers_del_timer(11);
	
	close_preprocessed_file(text_lines);
	sdk_prc_die();
}