#ifndef _UTIL_GUI_H_
#define _UTIL_GUI_H_

#include <sdk/scr/screen.h>

void gui_draw_header(Canvas * cv, const char * title);
void gui_draw_bottom(Canvas * cv, const char * bottom);
void gui_draw_area(Canvas * cv, const char * title, unsigned int x, unsigned int y, unsigned int width, unsigned int height);

#endif