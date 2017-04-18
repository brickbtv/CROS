#ifndef _EDITBOX_H_
#define _EDITBOX_H_

#include <oop.h>
#include <containers/list.h>
#include <sdk/scr/ScreenClass.h>
#include <utils/gui/gui.h>
#include <stddef_shared.h>

CLASS EditBoxClass{
	list_t * _text_lines;
	
	ScreenClass * _screen;
	
	Cursor _cursor;
	Cursor _cursor_prev_pos;
	
	int _insPress;
	bool _blink;
	bool _prev_blink;
	
	unsigned int _view_start_line;
	
	int _x;
	int _y;
	int _width;
	int _height;
	
	void (* set_list)(void * this, list_t * text_lines);
	list_t * (* get_list)(void * this);
	void (* handle_message)(void * this, int type, int reason, int value);
	
	void (* set_blink)(void * this, bool blink);
	
	void (* redraw)(void * this, int start_lint);
}EditBoxClass;

EditBoxClass * EditBoxClass_ctor(EditBoxClass * this, ScreenClass * screen, int x, int y, int height, int width);
void EditBoxClass_dtor(EditBoxClass * this);

#endif 