#include "editbox.h"

#include <utils/gui/gui.h>
#include <sdk/os/process.h>
#include <sdk/kyb/keyboard.h>
#include <sdk/os/debug.h>

void EditBoxClass_set(void * this, const char * text){
	
}

void EditBoxClass_set_list(void * this, list_t * text_lines){
	((EditBoxClass *)this)->_text_lines = text_lines;
	sdk_debug_logf("!! %s", list_at(text_lines, 0)->val);
}


char * EditBoxClass_get(void * this){
	
}


char * EditBoxClass_get_list(void * this){
	return ((EditBoxClass *)this)->_text_lines;
}

unsigned int text_cursor_y(EditBoxClass * this){
	return this->_cursor.y - 1 + this->_view_start_line;
}

char * current_line(EditBoxClass * this){
	return list_at(this->_text_lines, text_cursor_y(this))->val;
}

list_node_t * current_line_node(EditBoxClass * this){
	return list_at(this->_text_lines, text_cursor_y(this));
}

list_node_t * next_line_node(EditBoxClass * this){
	return list_at(this->_text_lines, text_cursor_y(this) + 1);
}

list_node_t * prev_line_node(EditBoxClass * this){
	return list_at(this->_text_lines, text_cursor_y(this) - 1);
}

void EditBoxClass_redraw(void * this, int start_line){
	EditBoxClass * editbox = (EditBoxClass *)this;
	
	editbox->_screen->clearArea(editbox->_screen, 
								SCR_COLOR_BLACK, 
								editbox->_x, 
								editbox->_y, 
								editbox->_width, 
								editbox->_height);
	
	for (int i = start_line; i < start_line + editbox->_screen->getScreenHeight(editbox->_screen)-2; i++){
		list_node_t* line = list_at(editbox->_text_lines, i);
		sdk_debug_logf("%s", line->val);
		if (line){
			editbox->_screen->printfXY(editbox->_screen, 0, i - start_line + 1, line->val);
		}
	}
	editbox->_view_start_line = start_line;
}


void EditBoxClass_handle_message(void * this_, int type, int reason, int value){
	EditBoxClass * this = (EditBoxClass *)this_;
	switch (type){
		case SDK_PRC_MESSAGE_CLK:
			//timers_handleMessage(type, reason, value);
			break;
		case SDK_PRC_MESSAGE_KYB: 
			// saving prev cursor pos
			this->_cursor_prev_pos.x = this->_cursor.x;
			this->_cursor_prev_pos.y = this->_cursor.y;
			
		
			// CTRL rplsmnt by INSERT
			if (value == KEY_INSERT){
				if (reason == KEY_STATE_KEYPRESSED)
					this->_insPress = 1;
				else if (reason == KEY_STATE_KEYRELEASED)
					this->_insPress = 0;
				
				break;
			}
			
			if (reason == KEY_STATE_KEYTYPED){
				if (this->_insPress == 1){
					if (value == KEY_UP){
						this->redraw(this, --this->_view_start_line);
						this->_cursor.y++;
						break;
					} 
					if (value == KEY_DOWN){
						this->redraw(this, ++this->_view_start_line);
						this->_cursor.y--;
						break;
					} 
				}
				
				if (this->_insPress == 1)
					break;
				
				if (value == KEY_BACKSPACE){
					if (this->_cursor.x > 0){
						this->_cursor.x--;
						strncpy(&current_line(this)[this->_cursor.x], 
								&current_line(this)[this->_cursor.x + 1], 
								strlen(current_line(this)) - this->_cursor.x);
						// redraw line
						this->_screen->printfXY(this->_screen, 0, text_cursor_y(this) + 1, "                                ");
						this->_screen->printfXY(this->_screen, 0, text_cursor_y(this) + 1, current_line(this));
					} else {	
						// cat current line to prevous
						if (this->_cursor.y > 1){
							char * prev_line = prev_line_node(this)->val;
							unsigned int new_cur_x = strlen(prev_line);
							
							strcpy(&prev_line[new_cur_x], current_line(this));
							list_remove(this->_text_lines, current_line_node(this));
							
							this->_cursor.x = new_cur_x;
							this->_cursor.y--;
							
							this->redraw(this, this->_view_start_line);
						}
					}
				} else if (value == KEY_DELETE){
					if (this->_cursor.x < strlen(current_line(this))){
						strncpy(&current_line(this)[this->_cursor.x], 
								&current_line(this)[this->_cursor.x + 1], 
								strlen(current_line(this)) - this->_cursor.x);
						// redraw line
						this->_screen->printfXY(this->_screen, 0, text_cursor_y(this) + 1, "                                ");
						this->_screen->printfXY(this->_screen, 0, text_cursor_y(this) + 1, current_line(this));
					} else {	
						// cat next line to current
						if (this->_cursor.y < list_size(this->_text_lines)){
							char * next_line = next_line_node(this)->val;
														
							strcpy(&current_line(this)[this->_cursor.x], next_line);
							list_remove(this->_text_lines, next_line_node(this));
														
							this->redraw(this, this->_view_start_line);
						}
					}
				} else if (value == KEY_RETURN){
					// shift text after
					// cut right part 
					char* new_line = (char*)calloc(strlen(&current_line(this)[this->_cursor.x]) + 1);
					strcpy(new_line, &current_line(this)[this->_cursor.x]);
					current_line(this)[this->_cursor.x] = 0;
					list_node_t* line_node = list_node_new(new_line);
					list_node_t* line_ins_after = list_at(this->_text_lines,this->_cursor.y + this->_view_start_line - 1);
					list_insertafter(this->_text_lines, line_ins_after, line_node);
					
					this->_cursor.y++;
					this->_cursor.x = 0;
					
					int offset = 0;
					if (this->_cursor.y > this->_screen->getScreenHeight(this->_screen) - 3){
						this->_view_start_line ++;
						this->_cursor.y --;
					}
					this->redraw(this, this->_view_start_line);		
				} else {
					if (value == KEY_UP){
						if (this->_cursor.y > 1){
							this->_cursor.y--;
							if (this->_cursor.x > strlen(current_line(this)))
								this->_cursor.x = strlen(current_line(this));
						} else {
							if (this->_cursor.y > 0 && this->_view_start_line > 0){
								redraw_text_area(--this->_view_start_line);
								this->_cursor.y ++;
							}
						}
					} 
					if (value == KEY_DOWN){
						if (this->_cursor.y < list_size(this->_text_lines) - this->_view_start_line)							
							this->_cursor.y++;
							if (this->_cursor.x > strlen(current_line(this)))
								this->_cursor.x = strlen(current_line(this));	
						
						if (this->_cursor.y > this->_screen->getScreenHeight(this->_screen) - 2 && text_cursor_y(this) < list_size(this->_text_lines) - 1){
							this->redraw(this, ++this->_view_start_line);
							this->_cursor.y --;
						}
					} 
					if (value == KEY_LEFT){
						if (this->_cursor.x > 0)
							this->_cursor.x--;
					} 
					if (value == KEY_RIGHT){
						if (this->_cursor.x < this->_screen->getScreenWidth(this->_screen) && this->_cursor.x < strlen(current_line(this)))
							this->_cursor.x++;
					} 					
					
					// letters
					if (value >= 0x20 && value <= 0x7E){						
						if (this->_cursor.x >= this->_screen->getScreenWidth(this->_screen) - 1 || strlen(current_line(this)) >= this->_screen->getScreenWidth(this->_screen)-1)
							break;
						
						// shift text
						char * cl = current_line(this);
						
						for (int i = strlen(cl); i >= this->_cursor.x && i >= 0; i--){
							cl[i + 1] = cl[i];
						}
						// add letter
						cl[this->_cursor.x] = value;
						this->_cursor.x++;
												
						// redraw line
						this->_screen->printfXY(this->_screen, 0, this->_cursor.y, cl);
					}
				}
			}
			break;
	}
	
	// redraw blinbking
	if (this->_cursor.x != this->_cursor_prev_pos.x || this->_cursor.y != this->_cursor_prev_pos.y){
		char pr_bl_char = ((char*)list_at(this->_text_lines, this->_cursor_prev_pos.y - 1 + this->_view_start_line)->val)[this->_cursor_prev_pos.x];
		if (pr_bl_char == 0)
			pr_bl_char = ' ';
		this->_screen->printfXY(this->_screen, this->_cursor_prev_pos.x, this->_cursor_prev_pos.y, "%c", pr_bl_char);
		this->_blink = true;
		this->_prev_blink = false;
	}
}

void EditBoxClass_set_blink(void * this_, bool blink){
	EditBoxClass * this = (EditBoxClass *) this_;
	this->_blink = blink;
	
	if (blink){
		this->_screen->setBackColor(this->_screen, SCR_COLOR_GREEN);
		this->_screen->setTextColor(this->_screen, SCR_COLOR_BLACK);
	} else {
		this->_screen->setBackColor(this->_screen, SCR_COLOR_BLACK);
	}
	
	char bl_char = current_line(this)[this->_cursor.x];
	if (bl_char == 0)
		bl_char = ' ';
		
	this->_screen->printfXY(this->_screen, this->_cursor.x, this->_cursor.y, "%c", bl_char);
	
	this->_screen->setBackColor(this->_screen, SCR_COLOR_BLACK);
	this->_screen->setTextColor(this->_screen, SCR_COLOR_GREEN);
	
	this->_prev_blink = this->_blink;
}

EditBoxClass * EditBoxClass_ctor(EditBoxClass * this, ScreenClass * screen, int x, int y, int height, int width){
	this->_x = x;
	this->_y = y;
	this->_height = height;
	this->_width = width;
	
	this->_screen = screen;
	
	this->_cursor.x = 0;
	this->_cursor.y = 1;
	this->_cursor_prev_pos.x = 0;
	this->_cursor_prev_pos.y = 1;
	
	this->_blink = false;
	this->_prev_blink = false;
	
	this->_view_start_line = 0;
	this->_insPress = 0;
	
	this->set = EditBoxClass_set;
	this->set_list = EditBoxClass_set_list;
	this->get = EditBoxClass_get;
	this->get_list = EditBoxClass_get_list;
	this->handle_message = EditBoxClass_handle_message;
	this->set_blink = EditBoxClass_set_blink;
	this->redraw = EditBoxClass_redraw;
}

void EditBoxClass_dtor(EditBoxClass * this){

}