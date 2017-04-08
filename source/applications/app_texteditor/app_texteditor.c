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

ScreenClass * screen;
GuiClass * gui;

list_t* text_lines;

Cursor cursor;
Cursor cursor_prev_pos;

bool preprocess_file(const char* path){
	/*
		Preprocessing split input file for \n symbols.
		Result stores in list.
	*/
		
	// step 1
	#define DEF_BUF_SIZE 256
	char buf[DEF_BUF_SIZE];
	memset(buf, 0, sizeof(char) * DEF_BUF_SIZE);
	unsigned int rb;
	
	FILE * file = fs_open_file(path, 'r');
	
	if (file){
		do{	
			int res = fs_read_file(file, buf, DEF_BUF_SIZE, &rb);
			if (res != FS_OK){
				list_destroy(text_lines);
				return FALSE;
			}
			
			// splitting
			int old_pos = -1;
			int pos = find(buf, '\n', 0);
			
			if (pos == -1){
				char* oneline = (char*)calloc(screen->getScreenWidth(screen));
				strcpy(oneline, buf);
				list_node_t* onl_node = list_node_new(oneline);
				list_rpush(text_lines, onl_node);
			}
			
			while (pos != -1){				
				char* line = (char*) calloc(screen->getScreenWidth(screen));
				strncpy(line, &buf[old_pos + 1], pos - old_pos - 1);
				
				list_rpush(text_lines, list_node_new(line));
				
				old_pos = pos;
				pos = find(buf, '\n', pos + 1);
			}
		} while(rb == DEF_BUF_SIZE);
	} else {
		return FALSE;
	}
	
	return TRUE;
}

unsigned int view_start_line = 0;

char path[256];

unsigned int text_cursor_y(){
	return cursor.y - 1 + view_start_line;
}

char * current_line(){
	return list_at(text_lines, text_cursor_y())->val;
}

list_node_t * current_line_node(){
	return list_at(text_lines, text_cursor_y());
}

list_node_t * next_line_node(){
	return list_at(text_lines, text_cursor_y() + 1);
}

list_node_t * prev_line_node(){
	return list_at(text_lines, text_cursor_y() - 1);
}

void redraw_text_area(int start_line){
	screen->clearScreen(screen, SCR_COLOR_BLACK);
	gui->draw_header(gui, path);
	gui->draw_bottom(gui, " INS^s - save    INS^x - quit");

	for (int i = start_line; i < start_line + screen->getScreenHeight(screen)-2; i++){
		list_node_t* line = list_at(text_lines, i);
		if (line)
			screen->printfXY(screen, 0, i - start_line + 1, line->val);
	}
	view_start_line = start_line;
}

int insPress = 0;
int exit = 0;
bool blink = false;
bool prev_blink = false;

void msgHandlerTexteditor(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_CLK:
			timers_handleMessage(type, reason, value);
			break;
		case SDK_PRC_MESSAGE_KYB: 
			// saving prev cursor pos
			cursor_prev_pos.x = cursor.x;
			cursor_prev_pos.y = cursor.y;
			
		
			// CTRL rplsmnt by INSERT
			if (value == KEY_INSERT){
				if (reason == KEY_STATE_KEYPRESSED)
					insPress = 1;
				else if (reason == KEY_STATE_KEYRELEASED)
					insPress = 0;
				
				break;
			}
			
			if (reason == KEY_STATE_KEYTYPED){
				if (insPress == 1){
					if (value == KEY_UP){
						redraw_text_area(--view_start_line);
						cursor.y++;
						break;
					} 
					if (value == KEY_DOWN){
						redraw_text_area(++view_start_line);
						cursor.y--;
						break;
					} 
				}
				
				if (value == KEY_BACKSPACE){
					if (cursor.x > 0){
						cursor.x--;
						strncpy(&current_line()[cursor.x], &current_line()[cursor.x + 1], strlen(current_line()) - cursor.x);
						// redraw line
						screen->printfXY(screen, 0, text_cursor_y() + 1, "                                ");
						screen->printfXY(screen, 0, text_cursor_y() + 1, current_line());
					} else {	
						// cat current line to prevous
						if (cursor.y > 1){
							char * prev_line = prev_line_node()->val;
							unsigned int new_cur_x = strlen(prev_line);
							
							strcpy(&prev_line[new_cur_x], current_line());
							list_remove(text_lines, current_line_node());
							
							cursor.x = new_cur_x;
							cursor.y--;
							
							redraw_text_area(view_start_line);
						}
					}
				} else if (value == KEY_DELETE){
					if (cursor.x < strlen(current_line())){
						strncpy(&current_line()[cursor.x], &current_line()[cursor.x + 1], strlen(current_line()) - cursor.x);
						// redraw line
						screen->printfXY(screen, 0, text_cursor_y() + 1, "                                ");
						screen->printfXY(screen, 0, text_cursor_y() + 1, current_line());
					} else {	
						// cat next line to current
						if (cursor.y < list_size(text_lines)){
							char * next_line = next_line_node()->val;
														
							strcpy(&current_line()[cursor.x], next_line);
							list_remove(text_lines, next_line_node());
														
							redraw_text_area(view_start_line);
						}
					}
				} else if (value == KEY_RETURN){
					// shift text after
					// cut right part 
					char* new_line = (char*)calloc(strlen(&current_line()[cursor.x]) + 1);
					strcpy(new_line, &current_line()[cursor.x]);
					current_line()[cursor.x] = 0;
					list_node_t* line_node = list_node_new(new_line);
					list_node_t* line_ins_after = list_at(text_lines, cursor.y + view_start_line - 1);
					list_insertafter(text_lines, line_ins_after, line_node);
					
					cursor.y++;
					cursor.x = 0;
					
					int offset = 0;
					if (cursor.y > screen->getScreenHeight(screen) - 3){
						view_start_line ++;
						cursor.y --;
					}
					redraw_text_area(view_start_line);		
				} else {
					// disable blinking for prevous cursor position
					//sdk_scr_setBackColor(cv, SCR_COLOR_BLACK);	
					//sdk_scr_printfXY(cv, cursor.x, cursor.y + view_start_line + 1, "%c", current_line()[cursor.x]);
				
					if (value == KEY_UP){
						if (cursor.y > 1){
							cursor.y--;
							if (cursor.x > strlen(current_line()))
								cursor.x = strlen(current_line());
						} else {
							if (cursor.y > 0 && view_start_line > 0){
								redraw_text_area(--view_start_line);
								cursor.y ++;
							}
						}
					} 
					if (value == KEY_DOWN){
						if (cursor.y < list_size(text_lines) - view_start_line)							
							cursor.y++;
							if (cursor.x > strlen(current_line()))
								cursor.x = strlen(current_line());	
						
						if (cursor.y > screen->getScreenHeight(screen) - 2 && text_cursor_y() < list_size(text_lines) - 1){
							redraw_text_area(++view_start_line);
							cursor.y --;
						}
					} 
					if (value == KEY_LEFT){
						if (cursor.x > 0)
							cursor.x--;
					} 
					if (value == KEY_RIGHT){
						if (cursor.x < screen->getScreenWidth(screen) && cursor.x < strlen(current_line()))
							cursor.x++;
					} 					
					
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
						
						if (cursor.x >= screen->getScreenWidth(screen) - 1 || strlen(current_line()) >= screen->getScreenWidth(screen)-1)
							break;
						
						// shift text
						char * cl = current_line();
						
						for (int i = strlen(cl); i >= cursor.x && i >= 0; i--){
							cl[i + 1] = cl[i];
						}
						// add letter
						cl[cursor.x] = value;
						cursor.x++;
												
						// redraw line
						screen->printfXY(screen, 0, cursor.y, cl);
					}
				}
			}
			break;
	}
	
	// redraw blinbking
	if (cursor.x != cursor_prev_pos.x || cursor.y != cursor_prev_pos.y){
		char pr_bl_char = ((char*)list_at(text_lines, cursor_prev_pos.y - 1 + view_start_line)->val)[cursor_prev_pos.x];
		if (pr_bl_char == 0)
			pr_bl_char = ' ';
		screen->printfXY(screen, cursor_prev_pos.x, cursor_prev_pos.y, "%c", pr_bl_char);
		blink = true;
		prev_blink = false;
	}
}

void teBlinkCBack(unsigned int tn){
	if (tn == 11){
		blink = !blink;
	}
}

void app_texteditor(const char* p){	
	Canvas * cv = (Canvas*)sdk_prc_getCanvas();
	
	cursor.x = 0;
	cursor.y = 1;
	cursor_prev_pos.x = 0;
	cursor_prev_pos.y = 1;
	
	exit = 0;
	view_start_line = 0;
	insPress = 0;

	text_lines = list_new();
	
	gui = malloc(sizeof(GuiClass));
	gui = GuiClass_ctor(gui, cv);
	
	screen = malloc(sizeof(ScreenClass));
	screen = ScreenClass_ctor(screen, cv);
		
	screen->clearScreen(screen, SCR_COLOR_BLACK);

	preprocess_file(p);
	memset(path, 0, sizeof(char) * 256);
	strcpy(path, p);
		
	gui->draw_header(gui, path);
	redraw_text_area(0);
	
	timers_add_timer(11, 500, teBlinkCBack);
	
	while(exit == 0){		
		if (prev_blink != blink){
			if (blink){
				screen->setBackColor(screen, SCR_COLOR_GREEN);
				screen->setTextColor(screen, SCR_COLOR_BLACK);
			} else {
				screen->setBackColor(screen, SCR_COLOR_BLACK);
			}
			
			char bl_char = current_line()[cursor.x];
			if (bl_char == 0)
				bl_char = ' ';
				
			screen->printfXY(screen, cursor.x, cursor.y, "%c", bl_char);
			
			screen->setBackColor(screen, SCR_COLOR_BLACK);
			screen->setTextColor(screen, SCR_COLOR_GREEN);
			
			prev_blink = blink;
		}
	
		if (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHandlerTexteditor);
		}
	}
	
	free(gui);
	free(screen);
	
	list_destroy(text_lines);
	sdk_prc_die();
}