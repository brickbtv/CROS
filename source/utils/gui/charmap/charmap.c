#include "charmap.h"

#include <stdlib/details/memdetails.h>
#include <sdk/os/debug.h>

GuiCharmap * gui_charmap_new(short * map, unsigned int xoffs, unsigned int yoffs, unsigned int width, unsigned int height){
	GuiCharmap * charmap = malloc(sizeof(GuiCharmap));
	charmap->map = map;
	charmap->xoffset = xoffs;
	charmap->yoffset = yoffs;
	charmap->width = width;
	charmap->height= height;
	
	charmap->cur.x = 0;
	charmap->cur.y = 0;
	
	charmap->prevcur.x = 0;
	charmap->prevcur.y = 0;
	return charmap;
}

void gui_charmap_redraw(GuiCharmap * charmap, Canvas * paint_canvas){
	for (int y = 0; y < charmap->height; y++)
		for (int x = 0; x < charmap->width; x++){
			short * canvas = paint_canvas->addr;
			*(canvas + (y + charmap->yoffset) * paint_canvas->res_hor + x + charmap->xoffset) = charmap->map[x + y * charmap->width];
		}
}

void gui_charmap_handleMessage(GuiCharmap * charmap, int value, char u, char d, char l, char r){
	if (value == u || value == d || value == l || value == r)
		charmap->blink = true;
		
	if (value == u){
		if (charmap->cur.y > 0){
			charmap->prevcur.x = charmap->cur.x;
			charmap->prevcur.y = charmap->cur.y;
			charmap->cur.y--;
		}
	}
	
	if (value == d){
		if (charmap->cur.y < charmap->height - 1){
			charmap->prevcur.x = charmap->cur.x;
			charmap->prevcur.y = charmap->cur.y;
			charmap->cur.y++; 
		}
	}
	
	if (value == l){
		if (charmap->cur.x > 0){
			charmap->prevcur.x = charmap->cur.x;
			charmap->prevcur.y = charmap->cur.y;
			charmap->cur.x--;
		}
	}
	
	if (value == r){
		if (charmap->cur.x < charmap->width - 1){
			charmap->prevcur.x = charmap->cur.x;
			charmap->prevcur.y = charmap->cur.y;
			charmap->cur.x++; 
		}
	}
}

void gui_charmap_draw_blink(GuiCharmap * charmap, Canvas * paint_canvas){
	short * canvas = paint_canvas->addr;
	unsigned char sel_ch = (unsigned char)charmap->map[charmap->cur.x + charmap->cur.y * charmap->width];
	
	short origin_char = charmap->map[charmap->cur.x + charmap->cur.y * charmap->width];
	
	char txt = origin_char >> 12;
	char bg = origin_char >> 8;
		
	short sel_ch_colored = bg << 12 | txt << 8 | sel_ch;
	
	*(canvas + (charmap->prevcur.y + charmap->yoffset) * paint_canvas->res_hor + charmap->prevcur.x + charmap->xoffset) = 
		charmap->map[charmap->prevcur.x + charmap->prevcur.y * charmap->width];
	*(canvas + (charmap->cur.y + charmap->yoffset) * paint_canvas->res_hor + charmap->cur.x + charmap->xoffset) = 
		charmap->blink ? sel_ch_colored : charmap->map[charmap->cur.x + charmap->cur.y * charmap->width];
}

short gui_charmap_get_symbol(GuiCharmap * charmap){
	return charmap->map[charmap->cur.x + charmap->cur.y * charmap->width];
}

short gui_charmap_set_symbol(GuiCharmap * charmap, unsigned int x, unsigned int y, short value){
	charmap->map[x + y * charmap->width] = value;
}