#include "app_texteditor.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/kyb/keyboard.h"
#include "sdk/clk/clock.h"

#include "filesystem/filesystem.h"

#include <string_shared.h>
#include <containers/list.h>

#define MAX_FILE_CHARS_PER_LINE 80

Canvas* cv;

list_t* text_lines;

#define LINE_LEN 80
#define LINES_COUNT 20

bool preprocess_file(const char* path){
	/*
		Preprocessing split input file for \n symbols.
		Result stores in list.
	*/
		
	// step 1
	#define DEF_BUF_SIZE 256
	char buf[DEF_BUF_SIZE];
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
				char* oneline = (char*)calloc(LINE_LEN);
				strcpy(oneline, buf);
				list_node_t* onl_node = list_node_new(oneline);
				list_rpush(text_lines, onl_node);
			}
			
			while (pos != -1){				
				char* line = (char*) calloc(LINE_LEN);
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

unsigned int cursor_x = 0;
unsigned int cursor_y = 0;

char path[256];

void draw_header(){
	sdk_scr_setBackColor(cv, SCR_COLOR_BLUE);
	
	for(int i = 0; i < LINE_LEN; i++)
		sdk_scr_putchar(cv, i, 0, ' ');

	sdk_scr_printfXY(cv, (LINE_LEN - strlen(path))/2, 0, "%s\n", path);
	
	sdk_scr_setBackColor(cv, SCR_COLOR_BLACK);
}

void redraw_text_area(int start_line){
	sdk_scr_clearScreen(cv, SCR_COLOR_BLACK);
	draw_header();

	for (int i = start_line; i < LINES_COUNT-1; i++){
		list_node_t* line = list_at(text_lines, i);
		if (line)
			sdk_scr_printfXY(cv, 0, i + 1, line->val);
	}
	view_start_line = start_line;
}

char * current_line(){
	return list_at(text_lines, cursor_y)->val;
}

list_node_t * current_line_node(){
	return list_at(text_lines, cursor_y);
}

list_node_t * next_line_node(){
	return list_at(text_lines, cursor_y + 1);
}


int insPress = 0;
int exit = 0;

void msgHandlerTexteditor(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_KYB: 
			// CTRL rplsmnt by INSERT
			if (value == KEY_INSERT){
				if (reason == KEY_STATE_KEYPRESSED)
					insPress  = 1;
				else if (reason == KEY_STATE_KEYRELEASED)
					insPress  = 0;
					
				break;
			}
		
			if (reason == KEY_STATE_KEYTYPED){
				if (value == KEY_BACKSPACE){
					if (cursor_x > 0){
						cursor_x--;
						strncpy(&current_line()[cursor_x], &current_line()[cursor_x + 1], strlen(current_line()) - cursor_x);
						// redraw line
						sdk_scr_printfXY(cv, 0, cursor_y + view_start_line + 1, "                                ");
						sdk_scr_printfXY(cv, 0, cursor_y + view_start_line + 1, current_line());
					} else {	
						// cat current line to prevous
						if (cursor_y > 0){
							char * prev_line = list_at(text_lines, cursor_y-1)->val;
							unsigned int new_cur_x = strlen(prev_line);
							
							strcpy(&prev_line[new_cur_x], current_line());
							list_remove(text_lines, current_line_node());
							
							cursor_x = new_cur_x;
							cursor_y--;
							
							redraw_text_area(view_start_line);
						}
					}
				} else if (value == KEY_DELETE){
					if (cursor_x < strlen(current_line())){
						strncpy(&current_line()[cursor_x], &current_line()[cursor_x + 1], strlen(current_line()) - cursor_x);
						// redraw line
						sdk_scr_printfXY(cv, 0, cursor_y + view_start_line + 1, "                                ");
						sdk_scr_printfXY(cv, 0, cursor_y + view_start_line + 1, current_line());
					} else {	
						// cat next line to current
						if (cursor_y < list_size(text_lines)){
							char * next_line = next_line_node()->val;
														
							strcpy(&current_line()[cursor_x], next_line);
							list_remove(text_lines, next_line_node());
														
							redraw_text_area(view_start_line);
						}
					}
				} else if (value == KEY_RETURN){
					// shift text after
					// cut right part 
					char* new_line = (char*)calloc(strlen(&current_line()[cursor_x]) + 1);
					strcpy(new_line, &current_line()[cursor_x]);
					current_line()[cursor_x] = 0;
					list_node_t* line_node = list_node_new(new_line);
					list_node_t* line_ins_after = list_at(text_lines, cursor_y);
					list_insertafter(text_lines, line_ins_after, line_node);
					
					cursor_y++;
					cursor_x = 0;
					
					redraw_text_area(view_start_line);					
				} else {
					// disable blinking for prevous cursor position
					sdk_scr_setBackColor(cv, SCR_COLOR_BLACK);	
					sdk_scr_printfXY(cv, cursor_x, cursor_y + view_start_line + 1, "%c", current_line()[cursor_x]);
				
					if (value == KEY_UP){
						if (cursor_y > 0){
							cursor_y--;
							if (cursor_x > strlen(current_line()))
								cursor_x = strlen(current_line());
						}	
					} 
					if (value == KEY_DOWN){
						if (cursor_y < list_size(text_lines) - 1)							
							cursor_y++;
							if (cursor_x > strlen(current_line()))
								cursor_x = strlen(current_line());	
					} 
					if (value == KEY_LEFT){
						if (cursor_x > 0)
							cursor_x--;
					} 
					if (value == KEY_RIGHT){
						if (cursor_x < 80 && cursor_x < strlen(current_line()))
							cursor_x++;
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
						
						// TODO: where is REALLOC????

						// shift text
						for (int i = strlen(current_line()); i >= cursor_x && i >= 0; i--){
							current_line()[i + 1] = current_line()[i];
						}				
						// add letter
						current_line()[cursor_x] = value;
						cursor_x++;
						sdk_debug_logf("%s", current_line());
						
						// redraw line
						sdk_scr_printfXY(cv, 0, cursor_y + view_start_line + 1, current_line());
					}
				}
			}
			break;
	}
}

void app_texteditor(const char* p){	
	text_lines = list_new();

	preprocess_file(p);
	strcpy(path, p);
	
	// interface	
	cv = (Canvas*)sdk_prc_getCanvas();
	sdk_scr_clearScreen(cv, SCR_COLOR_BLACK);
	
	draw_header();
	redraw_text_area(0);
	
	//
	bool prev_blink = false;
	bool blink = false;
	while(exit == 0){
		int time = sdk_clk_timeSinceBoot();
		if (time % 1000 < 500){
			blink = true;
		} else {
			blink = false;
		}
		if (prev_blink != blink){
			if (blink){
				sdk_scr_setBackColor(cv, SCR_COLOR_GREEN);
				sdk_scr_setTextColor(cv, SCR_COLOR_BLACK);
			} else {
				sdk_scr_setBackColor(cv, SCR_COLOR_BLACK);
			}
			
			char bl_char = current_line()[cursor_x];
			if (bl_char == 0)
				bl_char = ' ';
				
			sdk_scr_printfXY(cv, cursor_x, cursor_y + view_start_line + 1, "%c", bl_char);
			sdk_scr_setBackColor(cv, SCR_COLOR_BLACK);
			sdk_scr_setTextColor(cv, SCR_COLOR_GREEN);
			
			prev_blink = blink;
		}
		
		if (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHandlerTexteditor);
		}
	}
	exit = 0;
	view_start_line = 0;
	cursor_x = 0;
	cursor_y = 0;

	list_destroy(text_lines);
	
	sdk_debug_log("i'm done");
	sdk_prc_die();
}