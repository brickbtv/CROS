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
#include <stdlib/stdlib_shared.h>
#include <stdlib/stdio_shared.h>

Canvas * paint_canvas;
bool paint_run = true;
int state = 0;

GuiCharmap * p_canvas;
GuiCharmap * p_charmap;

GuiCharmap * p_main_color;
GuiCharmap * p_back_color;

Cursor bpcur, bprevcur;

unsigned int selected_color;

int paint_y_offset = 2;
int paint_x_offset = 1;

int paint_brush_y_offset = 2;
int paint_brush_x_offset = 1;

char canvas_size_input[20];

int width = 20;
int height = 10;
char paint_path[256];

char * open_file(const char * path, int * len){
	FILE * file = fs_open_file(path, 'r');
	if (!file)
		return NULL;
	char buf[256];
	int rb;
	char * picture = NULL;
	int size = -1;
	fs_read_file(file, buf, 256, &rb);
	while (rb > 0){
		sdk_debug_logf("rb: %d", rb);
		if (picture == NULL){
			picture = calloc(rb * sizeof(char));
			memcpy(picture, buf, rb);
			size += rb;
		} else {
			size += rb;
			short * tmp = calloc(size * sizeof(char));
			memcpy(tmp, picture, (size - rb) * sizeof(char));
			free(picture);
			picture = tmp;
			memcpy(&picture[size-rb], buf, rb * sizeof(char));
		}	
		fs_read_file(file, buf, 256, &rb);
	}
	*len = size;
	return picture;
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

int is_ins_pressed = 0;

void appPaintMsgHandler(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_CLK:
			timers_handleMessage(type, reason, value);
			break;
		case SDK_PRC_MESSAGE_KYB:
			if (state == 0){
				if (reason == KEY_STATE_KEYTYPED){
					if ((value >= '0' && value <= '9') || value == 'x'){
						canvas_size_input[strlen(canvas_size_input)] = value;
						sdk_scr_printf(paint_canvas, "%c", value);
						sdk_debug_logf("%s", canvas_size_input);
					}
					
					if (value == KEY_RETURN){
						int x_start_pos = find(canvas_size_input, 'x', 0);
						if (x_start_pos > 0){						
							char c_w[20];
							char c_h[20];
							strncpy(c_w, canvas_size_input, x_start_pos);
							strncpy(c_h, &canvas_size_input[x_start_pos + 1], strlen(canvas_size_input) - x_start_pos - 1);
							
							width = atoi(c_w);
							height = atoi(c_h);
							
							if (height == 0){
								sdk_scr_printf(paint_canvas, "\nIncorrect height. Try again:\n");
								memset(canvas_size_input, 0, 20);
								break;
							}
							
							state = 3;
						} else {
							sdk_scr_printf(paint_canvas, "\nCan't find 'x' symbol. Try again:\n");
							memset(canvas_size_input, 0, 20);
						}
					}
				}
				break;
			} else {
				// CTRL rplsmnt by INSERT
				if (value == KEY_INSERT){
					if (reason == KEY_STATE_KEYPRESSED)
						is_ins_pressed = 1;
					else if (reason == KEY_STATE_KEYRELEASED)
						is_ins_pressed = 0;
					
					break;
				}
				
				// saving
				if (value == 's' && is_ins_pressed == 1) {
					short * map = p_canvas->map;
					FILE * file = fs_open_file(paint_path, 'w');
					
					// header
					char size[20];
					sprintf(size, "%dx%d", width, height);
					fs_write_file(file, size);
					fs_write_file(file, ";");
					
					// body
					for (int i = 0; i < width * height; i++){
						short pixel = map[i];
						char low_byte = (char) pixel;
						char high_byte = pixel >> 8;
						char buf[2] = {high_byte, low_byte};
						fs_write_file(file, buf);
						sdk_debug_logf("%x %x %x", pixel, high_byte, low_byte);
					}
					fs_close_file(file);
				}
				
				if (value == 'x' && is_ins_pressed == 1){
					paint_run = 0;
				}
			}
			
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
	state = 0;
	paint_run = 1;
	memset(paint_path, 0, 256 * sizeof(char));
	strcpy(paint_path, path);
	
	/* state:
		0 - path is empty
		1 - path exists
		3 - entered new file size 
	*/
	
	if (strlen(path) == 0){
		state = 0;
	}
	
	short * map;
	
	int len = 0;
	char * pic = open_file(path, &len);
	sdk_debug_logf("LEN: %d", len);
	if (pic != NULL){
		// read format
		int size_pos = find(pic, ';', 0);
		sdk_debug_logf("LEN: %d", len);
				
		if (size_pos > 0){
			int x_start_pos = find(pic, 'x', 0);
			if (x_start_pos > 0){						
				char c_w[20];
				char c_h[20];
				strncpy(c_w, pic, x_start_pos);
				strncpy(c_h, &pic[x_start_pos + 1], size_pos - x_start_pos - 1);
				
				width = atoi(c_w);
				height = atoi(c_h);
				
				if (height == 0){
					sdk_debug_logf("\nIncorrect height. Try again:\n");
					///memset(pic, 0, 20);
				}
				
				map = calloc(width * height * sizeof(short));
				
				int count = 0;
				
				for (int i = size_pos + 1; i < len - 1; i+=5, count++){
					//sdk_debug_logf("%c", pic[i]);
					char high_byte = pic[i];
					char low_byte = pic[i+1];
					short pixel = high_byte << 8 | low_byte;
					map[count] = pixel;
					sdk_debug_logf("L:%d %d %d %d", count, high_byte, low_byte, pixel);
				}
				
				state = 3;
			} else {
				sdk_debug_logf("\nCan't find 'x' symbol. Try again:\n");
				////memset(pic, 0, 20);
			}
		
			//state = 1;
		} 
	}

	paint_canvas = (Canvas *)sdk_prc_getCanvas();
	sdk_scr_clearScreen(paint_canvas, SCR_COLOR_BLACK);

	if (state == 0){	// new picture
	
		width = 20;
		height = 10;
	
		sdk_scr_printfXY(paint_canvas, 0, 0, "Enter new file resolution in 'WIDTHxHEIGHT' format. Like '20x10':\n");
		while (state == 0){
			while (sdk_prc_haveNewMessage())
				sdk_prc_handleMessage(appPaintMsgHandler);
		}
		
		sdk_scr_clearScreen(paint_canvas, SCR_COLOR_BLACK);
	}
	
	timers_add_timer(2, 1000, paintBlinkCBack);
		
	paint_brush_x_offset = paint_canvas->res_hor - 34 + 1;
	
	int brushes_width = 32;
	int brushes_height = 8;
	
	memset(canvas_size_input, 0, 20);
	
	short * map_brushes = calloc(256 * sizeof(short));
	short * map_colors = calloc(16 * sizeof(short));
	short * map_back_colors = calloc(8 * sizeof(short));
	
	short back_ch = SCR_COLOR_BLUE << 12 | SCR_COLOR_WHITE << 8 | ' ';
	
	if (map == 0){
		map = calloc(width * height * sizeof(short));
	
		for (int i = 0; i < width * height; i++)
			map[i] = back_ch;
	}
		
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

	gui_draw_header(paint_canvas, "CROS Paint"/*path*/);
	gui_draw_bottom(paint_canvas, "    INS^s save    INS^x exit");
	
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
	sdk_prc_die();
}
