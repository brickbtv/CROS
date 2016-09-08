#include "app_paint.h"

#include <sdk/os/process.h>
#include <sdk/os/debug.h>
#include <sdk/scr/screen.h>
#include <sdk/kyb/keyboard.h>

#include <stdlib/details/memdetails.h>

#include <utils/filesystem/filesystem.h>
#include <utils/timers_and_clocks/timers.h>
#include <utils/gui/gui.h>
#include <utils/gui/charmap/charmap.h>

#include <stdlib/string_shared.h>

Canvas * paint_canvas;
bool paint_run = true;
int state = 0;

GuiCharmap * p_canvas;
GuiCharmap * p_charmap;

GuiCharmap * p_main_color;
GuiCharmap * p_back_color;

Cursor bpcur, bprevcur;

unsigned int selected_color

int paint_y_offset = 2;
int paint_x_offset = 1;

int paint_brush_y_offset = 2;
int paint_brush_x_offset = 1;

bool open_file(const char * path){
	FILE * file = fs_open_file(path, 'r');
	char buf[256];
	int rb;
	while (fs_read_file(file, buf, 256, &rb)){
		// TODO:
	}
}

void paintBlinkCBack(unsigned int tn){
	if (tn == 2){
		p_canvas->blink = !p_canvas->blink;
		p_charmap->blink = !p_charmap->blink;
		p_main_color->blink = !p_main_color->blink;
		p_back_color->blink = !p_back_color->blink;
		
		gui_charmap_draw_blink(p_canvas, paint_canvas);
		gui_charmap_draw_blink(p_charmap, paint_canvas);
		gui_charmap_draw_blink(p_back_color, paint_canvas);
		gui_charmap_draw_blink(p_main_color, paint_canvas);
	}
}

void appPaintMsgHandler(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_CLK:
			timers_handleMessage(type, reason, value);
			break;
		case SDK_PRC_MESSAGE_KYB:
			if (reason == KEY_STATE_KEYTYPED){				
				gui_charmap_handleMessage(p_canvas, value, 'w', 's', 'a', 'd');
				gui_charmap_handleMessage(p_charmap, value, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT);
				gui_charmap_handleMessage(p_main_color, value, 'w', 's', 'i', 'o');
				gui_charmap_handleMessage(p_back_color, value, 'w', 's', 'k', 'l');
				
				gui_charmap_draw_blink(p_canvas, paint_canvas);
				gui_charmap_draw_blink(p_charmap, paint_canvas);
				gui_charmap_draw_blink(p_back_color, paint_canvas);
				gui_charmap_draw_blink(p_main_color, paint_canvas);
				
				if (value == ' '){
					unsigned char value = (unsigned char)gui_charmap_get_symbol(p_charmap);
					char main_color = (gui_charmap_get_symbol(p_main_color) >> 8) % 16;
					char back_color = gui_charmap_get_symbol(p_back_color) >> 12;
										
					short new_val = (back_color << 12) | (main_color << 8) | value;					
					gui_charmap_set_symbol(p_canvas, p_canvas->cur.x, p_canvas->cur.y, new_val);
				}
			}
			break;
	}
}

void app_paint(const char * path){
	timers_add_timer(2, 1000, paintBlinkCBack);
	
	paint_canvas = (Canvas *)sdk_prc_getCanvas();
	
	paint_brush_x_offset = paint_canvas->res_hor - 34 + 1;

	int width = 20;
	int height = 10;
	
	int brushes_width = 32;
	int brushes_height = 8;
	
	short * map = calloc(width * height * sizeof(short));
	short * map_brushes = calloc(256 * sizeof(short));
	short * map_colors = calloc(16 * sizeof(short));
	short * map_back_colors = calloc(8 * sizeof(short));
	
	short back_ch = SCR_COLOR_BLUE << 12 | SCR_COLOR_WHITE << 8 | ' ';
	
	for (int i = 0; i < width * height; i++)
		map[i] = back_ch;
		
	for (short i = 0; i < 256; i++)
		map_brushes[i] = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | i;
		
	for (short i = 0; i < 16; i++){
		char color = i;
		if (color == SCR_COLOR_WHITE)
			color = SCR_COLOR_BLACK;
		map_colors[i] = SCR_COLOR_WHITE << 12 | color << 8 | 8;
	}
	
	for (short i = 0; i < 8; i++){
		char color = i;
		if (color == SCR_COLOR_RED)
			color = SCR_COLOR_BLUE;
		map_back_colors[i] = i << 12 | (8-color) << 8 | 7;
	}
	
	p_canvas = gui_charmap_new(map, paint_x_offset, paint_y_offset, width, height);	
	p_charmap = gui_charmap_new(map_brushes, paint_brush_x_offset, paint_brush_y_offset, brushes_width, brushes_height);
	p_main_color = gui_charmap_new(map_colors, paint_brush_x_offset + 8, 1 + brushes_height + 3, 16, 1);
	p_back_color = gui_charmap_new(map_back_colors, paint_brush_x_offset + 8, 1 + brushes_height + 5, 8, 1);
	
	// selecting default colors
	p_main_color->cur.x = 7;
	
	
	sdk_scr_clearScreen(paint_canvas, SCR_COLOR_BLACK);

	gui_draw_header(paint_canvas, "CROS Paint"/*path*/);
	gui_draw_bottom(paint_canvas, "");
	
	gui_draw_area(paint_canvas, "Canvas", 0, 1, width + 2, height + 2);
	gui_draw_area(paint_canvas, "Brushes", paint_canvas->res_hor - brushes_width - 2, 1, brushes_width + 2, brushes_height + 2);
	gui_draw_area(paint_canvas, "Colors", paint_canvas->res_hor - brushes_width - 2, 1 + brushes_height + 2, brushes_width + 2, 5);
	
	gui_charmap_redraw(p_canvas, paint_canvas);
	gui_charmap_redraw(p_charmap, paint_canvas);
	
	sdk_scr_printfXY(paint_canvas, paint_canvas->res_hor - brushes_width + 1, 1 + brushes_height + 3, "Main: ");
	sdk_scr_printfXY(paint_canvas, paint_canvas->res_hor - brushes_width + 1, 1 + brushes_height + 5, "Back: ");
	gui_charmap_redraw(p_main_color, paint_canvas);
	gui_charmap_redraw(p_back_color, paint_canvas);
	
	while (paint_run){
		while (sdk_prc_haveNewMessage())
			sdk_prc_handleMessage(appPaintMsgHandler);
	}
}
