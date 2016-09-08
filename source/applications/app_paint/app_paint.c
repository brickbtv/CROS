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
unsigned int width, height;
static bool blink = true;

typedef struct PCursor{
	unsigned int x; 
	unsigned int y;
}PCursor;

PCursor pcur, prevcur;

int paint_y_offset = 2;
int paint_x_offset = 1;

bool open_file(const char * path){
	FILE * file = fs_open_file(path, 'r');
	char buf[256];
	int rb;
	while (fs_read_file(file, buf, 256, &rb)){
		// TODO:
	}
}

void draw_blink(){
	short * canvas = paint_canvas->addr;
	char sel_ch = (char)map[pcur.x + pcur.y * width];
	
	short origin_char = map[pcur.x + pcur.y * width];
	
	char txt = origin_char >> 12;
	char bg = origin_char >> 8;
		
	short sel_ch_colored = bg << 12 | txt << 8 | sel_ch;
	
	*(canvas + (prevcur.y + paint_y_offset) * paint_canvas->res_hor + prevcur.x + paint_x_offset) = map[prevcur.x + prevcur.y * width];
	*(canvas + (pcur.y + paint_y_offset) * paint_canvas->res_hor + pcur.x + paint_x_offset) = blink?sel_ch_colored:map[pcur.x + pcur.y * width];
}

void paintBlinkCBack(unsigned int tn){
	if (tn == 2){
		blink = ! blink;
		draw_blink();
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
				
				if (value == KEY_UP || value == KEY_DOWN || value == KEY_LEFT || value == KEY_RIGHT)
					blink = true;
				
				if (value == KEY_UP){
					if (pcur.y > 0){
						prevcur.x = pcur.x;
						prevcur.y = pcur.y;
						pcur.y--;
					}
				}
				
				if (value == KEY_DOWN){
					if (pcur.y < height - 1){
						prevcur.x = pcur.x;
						prevcur.y = pcur.y;
						pcur.y++; 
					}
				}
				
				if (value == KEY_LEFT){
					if (pcur.x > 0){
						prevcur.x = pcur.x;
						prevcur.y = pcur.y;
						pcur.x--;
					}
				}
				
				if (value == KEY_RIGHT){
					if (pcur.x < width - 1){
						prevcur.x = pcur.x;
						prevcur.y = pcur.y;
						pcur.x++; 
					}
				}				

				draw_blink();
			}
			break;
	}
}

void app_paint(const char * path){
	timers_add_timer(2, 500, paintBlinkCBack);
	
	paint_canvas = (Canvas *)sdk_prc_getCanvas();
		
	pcur.x = 0;
	pcur.y = 0;
	
	prevcur.x = 0;
	prevcur.y = 0;
	
	width = 20;
	height = 10;
	map = calloc(width * height * sizeof(short));
	
	short back_ch = SCR_COLOR_BLUE << 12 | SCR_COLOR_WHITE << 8 | ',';
	
	for (int i = 0; i < width * height; i++)
		map[i] = back_ch;
	
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
