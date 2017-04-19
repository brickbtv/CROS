#include "editbox.h"

#include <utils/gui/gui.h>
#include <sdk/os/process.h>
#include <sdk/kyb/keyboard.h>
#include <sdk/os/debug.h>

void EditBoxClass_set_list(void * this, list_t * text_lines){
	((EditBoxClass *)this)->_text_lines = text_lines;
}

char * EditBoxClass_get_list(void * this){
	return ((EditBoxClass *)this)->_text_lines;
}

unsigned int cursor_to_screen_y(EditBoxClass * this){
	return this->_cursor.y + this->_y - this->_view_start_line;
}

unsigned int cursor_to_screen_x(EditBoxClass * this){
	return this->_cursor.x + this->_x;
}

char * current_line(EditBoxClass * this){
	return list_at(this->_text_lines, this->_cursor.y)->val;
}

list_node_t * current_line_node(EditBoxClass * this){
	return list_at(this->_text_lines, this->_cursor.y);
}

list_node_t * next_line_node(EditBoxClass * this){
	return list_at(this->_text_lines, this->_cursor.y + 1);
}

list_node_t * prev_line_node(EditBoxClass * this){
	return list_at(this->_text_lines, this->_cursor.y - 1);
}

void draw_line(EditBoxClass * this, int i, char * line, int start_line){
	char * buf = (char * )line;
	char temp_char = 0;
	if (strlen(line) > this->_width){
		temp_char = buf[this->_width];
		buf[this->_width] = 0;
	}
	this->_screen->printfXY(this->_screen, this->_x, i - start_line + this->_y, buf);

	if (temp_char != 0)
		buf[this->_width] = temp_char;
}

void EditBoxClass_redraw(void * this, int start_line){
	EditBoxClass * editbox = (EditBoxClass *)this;
	
	editbox->_screen->clearArea(editbox->_screen, 
								SCR_COLOR_BLACK, 
								editbox->_x, 
								editbox->_y, 
								editbox->_width, 
								editbox->_height);
	
	for (int i = start_line; i < start_line + editbox->_height; i++){
		list_node_t* line = list_at(editbox->_text_lines, i);
		if (line)
			draw_line(this, i, (char *)line->val, start_line);
	}
	editbox->_view_start_line = start_line;
}

void update_insert_key_status(EditBoxClass * this, int reason){
	if (reason == KEY_STATE_KEYPRESSED)
		this->_insPress = 1;
	else if (reason == KEY_STATE_KEYRELEASED)
		this->_insPress = 0;
}

void redraw_current_line(EditBoxClass * this){
	this->_screen->clearArea(this->_screen, 
							SCR_COLOR_BLACK, 
							this->_x, cursor_to_screen_y(this), 
							this->_width, 
							1);
	this->_screen->printfXY(this->_screen, 
							this->_x, cursor_to_screen_y(this), 
							current_line(this));
}

void process_backspace(EditBoxClass * this){
	if (this->_cursor.x > 0){
		this->_cursor.x--;
		strncpy(&current_line(this)[this->_cursor.x], 
				&current_line(this)[this->_cursor.x + 1], 
				strlen(current_line(this)) - this->_cursor.x);
		
		redraw_current_line(this);
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
}

void process_delete(EditBoxClass * this){
	if (this->_cursor.x < strlen(current_line(this))){
		strncpy(&current_line(this)[this->_cursor.x], 
				&current_line(this)[this->_cursor.x + 1], 
				strlen(current_line(this)) - this->_cursor.x);
		
		redraw_current_line(this);
	} else {	
		// cat next line to current
		if (this->_cursor.y < list_size(this->_text_lines)){
			char * next_line = next_line_node(this)->val;
										
			strcpy(&current_line(this)[this->_cursor.x], next_line);
			list_remove(this->_text_lines, next_line_node(this));
										
			this->redraw(this, this->_view_start_line);
		}
	}
}

void process_return(EditBoxClass * this){
	char* new_line = (char*)calloc(82);
	strcpy(new_line, &current_line(this)[this->_cursor.x]);
	
	current_line(this)[this->_cursor.x] = 0;
	
	list_node_t* line_node = list_node_new(new_line);
	list_node_t* line_ins_after = list_at(	this->_text_lines,
											this->_cursor.y + this->_view_start_line);
	list_insertafter(this->_text_lines, line_ins_after, line_node);

	this->_cursor.y++;
	this->_cursor.x = 0;
	
	if (this->_cursor.y > this->_height - 1){
		this->_view_start_line ++;
		this->_cursor.y --;
	}
	this->redraw(this, this->_view_start_line);	
}

void cursor_up(EditBoxClass * this){
	if (this->_cursor.y > 0){
		this->_cursor.y--;
		if (this->_cursor.x > strlen(current_line(this)))
			this->_cursor.x = strlen(current_line(this));
	} 
	
	if (this->_cursor.y < this->_view_start_line){
		redraw_text_area(--this->_view_start_line);
	}
	
}

void cursor_down(EditBoxClass * this){
	if (this->_cursor.y + 1 < list_size(this->_text_lines)){
		this->_cursor.y++;	
	} 
	if (this->_cursor.y - this->_view_start_line > (this->_height - 1)){
		this->redraw(this, ++this->_view_start_line);
	}
	
	if (this->_cursor.x > strlen(current_line(this)))
		this->_cursor.x = strlen(current_line(this));
}

void cursor_left(EditBoxClass * this){
	if (this->_cursor.x > 0)
		this->_cursor.x--;
}

void cursor_right(EditBoxClass * this){
	int lenght = strlen(current_line(this));
	if (this->_cursor.x < this->_width && this->_cursor.x < lenght)
		this->_cursor.x++;
}

void process_new_char(EditBoxClass * this, char value){
	if (this->_cursor.x >= this->_width - 1 
		|| strlen(current_line(this)) >= this->_width-1)
		return;

	// shift text
	char * cl = current_line(this);

	for (int i = strlen(cl); i >= this->_cursor.x && i >= 0; i--)
		cl[i + 1] = cl[i];
	
	// add letter
	cl[this->_cursor.x] = value;
	this->_cursor.x++;
							
	// redraw line
	this->_screen->printfXY(this->_screen, this->_x, cursor_to_screen_y(this), cl);
}

bool is_cursor_moved(EditBoxClass * this){
	return 	this->_cursor.x != this->_cursor_prev_pos.x 
			|| this->_cursor.y != this->_cursor_prev_pos.y;
}

void redraw_blinking(EditBoxClass * this){
	if (is_cursor_moved(this)){
		char * prev_line = (char*)list_at(	this->_text_lines, 
											this->_cursor_prev_pos.y/* - this->_view_start_line*/) -> val;
											
		char pr_bl_char = prev_line[this->_cursor_prev_pos.x];
		if (pr_bl_char == 0)
			pr_bl_char = ' ';
			
		this->_screen->printfXY(this->_screen, 
								this->_cursor_prev_pos.x + this->_x, 
								this->_cursor_prev_pos.y + this->_y - this->_view_start_line, 
								"%c", 
								pr_bl_char);
								
		this->set_blink(this, true);
	}
}

void EditBoxClass_handle_message(void * this_, int type, int reason, int value){
	EditBoxClass * this = (EditBoxClass *)this_;
	switch (type){
		case SDK_PRC_MESSAGE_KYB: 
			// saving prev cursor pos
			this->_cursor_prev_pos.x = this->_cursor.x;
			this->_cursor_prev_pos.y = this->_cursor.y;
			
		
			// CTRL rplsmnt by INSERT
			if (value == KEY_INSERT){
				update_insert_key_status(this, reason);
				
				break;
			}
			
			if (reason == KEY_STATE_KEYTYPED){
				
				if (value == KEY_BACKSPACE)
					process_backspace(this);
				if (value == KEY_DELETE)
					process_delete(this);
				if (value == KEY_RETURN)
					process_return(this);	
				if (value == KEY_UP)
					cursor_up(this);
				if (value == KEY_DOWN)
					cursor_down(this);
				if (value == KEY_LEFT)
					cursor_left(this);
				if (value == KEY_RIGHT)
					cursor_right(this);
				 					
				// letters
				if (this->_insPress == false)
					if (value >= 0x20 && value <= 0x7E)
						process_new_char(this, value);
				
				redraw_blinking(this);
			}
			
			break;
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
		
	this->_screen->printfXY(this->_screen, 
							cursor_to_screen_x(this), 
							cursor_to_screen_y(this), 
							"%c", bl_char);
	
	this->_screen->setBackColor(this->_screen, SCR_COLOR_BLACK);
	this->_screen->setTextColor(this->_screen, SCR_COLOR_GREEN);
	
	this->_prev_blink = this->_blink;
}

EditBoxClass * EditBoxClass_ctor(	EditBoxClass * this, 
									ScreenClass * screen, 
									int x, int y, 
									int height, int width){
	this->_x = x;
	this->_y = y;
	this->_height = height;
	this->_width = width;
	
	this->_screen = screen;
	
	this->_cursor.x = 0;
	this->_cursor.y = 0;
	this->_cursor_prev_pos.x = 0;
	this->_cursor_prev_pos.y = 0;
	
	this->_blink = false;
	this->_prev_blink = false;
	
	this->_view_start_line = 0;
	this->_insPress = 0;
	
	this->set_list = EditBoxClass_set_list;
	this->get_list = EditBoxClass_get_list;
	this->handle_message = EditBoxClass_handle_message;
	this->set_blink = EditBoxClass_set_blink;
	this->redraw = EditBoxClass_redraw;
}

void EditBoxClass_dtor(EditBoxClass * this){

}