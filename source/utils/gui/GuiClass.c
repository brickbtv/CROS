#include "GuiClass.h"
#include "gui.h"


void gui_class_draw_header(void * this, const char * title){
	gui_draw_header(((GuiClass*)this)->_canvas, title); 
}

void gui_class_draw_bottom(void * this, const char * bottom){
	gui_draw_bottom(((GuiClass*)this)->_canvas, bottom);
}

void gui_class_draw_area(void * this, const char * title, unsigned int x, unsigned int y, unsigned int width, unsigned int height){
	gui_draw_area(((GuiClass*)this)->_canvas, title, x, y, width, height);
}

GuiClass * GuiClass_ctor(GuiClass * this, Canvas * canvas){
	this->_canvas = canvas;
	this->draw_header = gui_class_draw_header;
	this->draw_bottom = gui_class_draw_bottom;
	this->draw_area = gui_class_draw_area;
	
	return this;
}
