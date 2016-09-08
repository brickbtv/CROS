#ifndef _UTIL_GUI_CHARMAP_H_
#define _UTIL_GUI_CHARMAP_H_

#include "../gui.h"
#include <stdlib/stddef_shared.h>

typedef struct GuiCharmap{
	short * map;
	Cursor cur, prevcur;
	unsigned int xoffset, yoffset;
	unsigned int width, height;
	bool blink;
} GuiCharmap;

GuiCharmap * gui_charmap_new(short * map, unsigned int xoffs, unsigned int yoffs, unsigned int width, unsigned int height);
void gui_charmap_redraw(GuiCharmap * charmap, Canvas * paint_canvas);
void gui_charmap_handleMessage(GuiCharmap * charmap, int value, char u, char d, char l, char r);
void gui_charmap_draw_blink(GuiCharmap * charmap, Canvas * paint_canvas);
short gui_charmap_get_symbol(GuiCharmap * charmap);
short gui_charmap_set_symbol(GuiCharmap * charmap, unsigned int x, unsigned int y, short value);

#endif