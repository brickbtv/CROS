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

Cursor bpcur, bprevcur;

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
		gui_charmap_draw_blink(p_canvas, paint_canvas);
		gui_charmap_draw_blink(p_charmap, paint_canvas);
	}
}

void appPaintMsgHandler(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_CLK:
			timers_handleMessage(type, reason, value);
			break;
		case SDK_PRC_MESSAGE_KYB:
			if (reason == KEY_STATE_KEYTYPED){
				if (state < 2){
					if (value >= '0' && value <= '9'){
						
					}
				}
				
				gui_charmap_handleMessage(p_canvas, value, 'w', 's', 'a', 'd');
				gui_charmap_handleMessage(p_charmap, value, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT);
				
				gui_charmap_draw_blink(p_canvas, paint_canvas);
				gui_charmap_draw_blink(p_charmap, paint_canvas);
			}
			break;
	}
}

void app_paint(const char * path){
	timers_add_timer(2, 500, paintBlinkCBack);
	
	paint_canvas = (Canvas *)sdk_prc_getCanvas();
	
	paint_brush_x_offset = paint_canvas->res_hor - 34 + 1;

	int width = 20;
	int height = 10;
	
	int brushes_width = 32;
	int brushes_height = 8;
	
	short * map = calloc(width * height * sizeof(short));
	short * map_brushes = calloc(256 * sizeof(short));
	
	short back_ch = SCR_COLOR_BLUE << 12 | SCR_COLOR_WHITE << 8 | ',';
	
	for (int i = 0; i < width * height; i++)
		map[i] = back_ch;
		
	for (short i = 0; i < 256; i++)
		map_brushes[i] = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | i;
	
	p_canvas = gui_charmap_new(map, paint_x_offset, paint_y_offset, width, height);	
	p_charmap = gui_charmap_new(map_brushes, paint_brush_x_offset, paint_brush_y_offset, brushes_width, brushes_height);
	
	sdk_scr_clearScreen(paint_canvas, SCR_COLOR_BLACK);

	gui_draw_header(paint_canvas, "CROS Paint"/*path*/);
	gui_draw_bottom(paint_canvas, "");
	
	gui_draw_area(paint_canvas, "Canvas", 0, 1, width + 2, height + 2);
	gui_draw_area(paint_canvas, "Brushes", paint_canvas->res_hor - brushes_width - 2, 1, brushes_width + 2, brushes_height + 2);
	
	gui_charmap_redraw(p_canvas, paint_canvas);
	gui_charmap_redraw(p_charmap, paint_canvas);
	
	while (paint_run){
		while (sdk_prc_haveNewMessage())
			sdk_prc_handleMessage(appPaintMsgHandler);
	}
}
