#include "app_paint.h"

#include <sdk/os/process.h>
#include <sdk/os/debug.h>
#include <sdk/scr/screen.h>
#include <sdk/kyb/keyboard.h>

#include <stdlib/details/memdetails.h>

#include <utils/filesystem/filesystem.h>
#include <utils/timers_and_clocks/timers.h>
#include <utils/gui/gui.h>

#include <stdlib/string_shared.h>

Canvas * paint_canvas;
bool paint_run = true;
int state = 0;
short * map;
short * map_brushes;
unsigned int width, height;
static bool blink = true;

typedef struct PCursor{
	unsigned int x; 
	unsigned int y;
}PCursor;

PCursor pcur, prevcur;
PCursor bpcur, bprevcur;

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

void draw_blink(short * palette, PCursor * c, PCursor * prevc, int xoffset, int yoffset, int wid){
	short * canvas = paint_canvas->addr;
	char sel_ch = (char)palette[c->x + c->y * wid];
	
	short origin_char = palette[c->x + c->y * wid];
	
	char txt = origin_char >> 12;
	char bg = origin_char >> 8;
		
	short sel_ch_colored = bg << 12 | txt << 8 | sel_ch;
	
	*(canvas + (prevc->y + yoffset) * paint_canvas->res_hor + prevc->x + xoffset) = palette[prevc->x + prevc->y * wid];
	*(canvas + (c->y + yoffset) * paint_canvas->res_hor + c->x + xoffset) = blink?sel_ch_colored:palette[c->x + c->y * wid];
}

void paintBlinkCBack(unsigned int tn){
	if (tn == 2){
		blink = ! blink;
		draw_blink(map, &pcur, &prevcur, paint_x_offset, paint_y_offset, width);
		draw_blink(map_brushes, &bpcur, &bprevcur, paint_brush_x_offset, paint_brush_y_offset, 32);
	}
}

void redraw(){
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++){
			short * canvas = paint_canvas->addr;
			*(canvas + (y + paint_y_offset) * paint_canvas->res_hor + x + paint_x_offset) = map[x + y * width];
		}
}

void draw_brushes(){
	for (short i = 0; i < 256; i++){
		*(paint_canvas->addr + (paint_canvas->res_hor * (2 + i / 32)  + 1) + paint_canvas->res_hor - 34 + i % 32) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | i;
	}
}

void paintUpdateCursors(int value, char u, char d, char l, char r, int max_width, int max_height, PCursor * c, PCursor * prev){
	if (value == KEY_UP || value == KEY_DOWN || value == KEY_LEFT || value == KEY_RIGHT)
		blink = true;
	
	if (value == u){
		if (c->y > 0){
			prev->x = c->x;
			prev->y = c->y;
			c->y--;
		}
	}
	
	if (value == d){
		if (c->y < max_height - 1){
			prev->x = c->x;
			prev->y = c->y;
			c->y++; 
		}
	}
	
	if (value == l){
		if (c->x > 0){
			prev->x = c->x;
			prev->y = c->y;
			c->x--;
		}
	}
	
	if (value == r){
		if (c->x < max_width - 1){
			prev->x = c->x;
			prev->y = c->y;
			c->x++; 
		}
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
				
				paintUpdateCursors(value, 'w', 's', 'a', 'd', width, height, &pcur, &prevcur);
				paintUpdateCursors(value, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 32, 8, &bpcur, &bprevcur);

				draw_blink(map, &pcur, &prevcur, paint_x_offset, paint_y_offset, width);
				draw_blink(map_brushes, &bpcur, &bprevcur, paint_brush_x_offset, paint_brush_y_offset, 32);
			}
			break;
	}
}

void app_paint(const char * path){
	timers_add_timer(2, 500, paintBlinkCBack);
	
	paint_canvas = (Canvas *)sdk_prc_getCanvas();
		
	paint_brush_x_offset = paint_canvas->res_hor - 34 + 1;
		
	pcur.x = 0;
	pcur.y = 0;
	
	prevcur.x = 0;
	prevcur.y = 0;
	
	bpcur.x = 0;
	bpcur.y = 0;
	
	bprevcur.x = 0;
	bprevcur.y = 0;
	
	width = 20;
	height = 10;
	map = calloc(width * height * sizeof(short));
	map_brushes = calloc(256 * sizeof(short));
	
	short back_ch = SCR_COLOR_BLUE << 12 | SCR_COLOR_WHITE << 8 | ',';
	
	for (int i = 0; i < width * height; i++)
		map[i] = back_ch;
		
	for (short i = 0; i < 256; i++)
		map_brushes[i] = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | i;
	
	sdk_scr_clearScreen(paint_canvas, SCR_COLOR_BLACK);
	//sdk_scr_printf(paint_canvas, "WOoops");
	
	gui_draw_header(paint_canvas, "CROS Paint"/*path*/);
	gui_draw_bottom(paint_canvas, "");
	
	gui_draw_area(paint_canvas, "Canvas", 0, 1, 22, 12);
	gui_draw_area(paint_canvas, "Brushes", paint_canvas->res_hor - 34, 1, 34, 10);
	redraw();
	draw_brushes();
	
	while (paint_run){
		while (sdk_prc_haveNewMessage())
			sdk_prc_handleMessage(appPaintMsgHandler);
	}
}
