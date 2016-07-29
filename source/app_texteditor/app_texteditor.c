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
			int old_pos = 0;
			int pos = find(buf, '\n', 0);
			
			if (pos == -1){
				char* oneline = (char*)malloc(strlen(buf));
				strcpy(oneline, buf);
				list_node_t* onl_node = list_node_new(oneline);
				list_rpush(text_lines, onl_node);
			}
			
			while (pos != -1){
				char* line = (char*) malloc(pos + 1);
				
				strncpy(line, &buf[old_pos], pos);
				
				list_node_t* line_node = list_node_new(line);
				list_rpush(text_lines, line_node);
				
				old_pos = pos;
				pos = find(buf, '\n', pos);
			}
		} while(rb == DEF_BUF_SIZE);
	} else {
		return FALSE;
	}
	
	for (int i = 0; i < text_lines->len; i++){
		sdk_debug_logf("text: %s", list_at(text_lines, i));
	}
	
	return TRUE;
}

unsigned int view_start_line = 0;

unsigned int cursor_x = 0;
unsigned int cursor_y = 0;

char path[256];

void draw_header(){
	sdk_scr_setBackColor(cv, SCR_COLOR_BLUE);
	
	for(int i = 0; i < 80; i++)
		sdk_scr_putchar(cv, i, 0, ' ');

	sdk_scr_printfXY(cv, (80 - strlen(path))/2, 0, "%s\n", path);
	
	sdk_scr_setBackColor(cv, SCR_COLOR_BLACK);
}

void redraw_text_area(int start_line){
	sdk_scr_clearScreen(cv, SCR_COLOR_BLACK);
	draw_header();

	for (int i = start_line; i < 20-1; i++){
		list_node_t* line = list_at(text_lines, i);
		if (line)
			sdk_scr_printfXY(cv, 0, i + 1, line->val);
	}
	view_start_line = start_line;
}

char * current_line(){
	return list_at(text_lines, cursor_y)->val;
}

void msgHandlerTexteditor(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_KYB: 
			if (reason == KEY_STATE_KEYTYPED){
				if (value == KEY_BACKSPACE){
					
					if (cursor_x > 0)
						cursor_x--;
				} else if (value == KEY_RETURN){
					// shift text after
					// cut right part 
					char* new_line = (char*)malloc(strlen(&current_line()[cursor_x]) + 1);
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
	while(1){
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
}