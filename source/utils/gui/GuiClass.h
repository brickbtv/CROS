#ifndef _GUI_CLASS_H_
#define _GUI_CLASS_H_

#include <sdk/scr/screen.h>

typedef struct GuiClass{
	Canvas * _canvas;
		
	void (* draw_header)(void * this, const char * title);
	void (* draw_bottom)(void * this, const char * bottom);
	void (* draw_area)  (void * this, const char * title, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
} GuiClass;


GuiClass * GuiClass_ctor(GuiClass * this, Canvas * canvas);

#endif