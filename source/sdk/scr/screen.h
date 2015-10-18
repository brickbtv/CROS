#ifndef _SCR_SCREEN_H_
#define _SCR_SCREEN_H_

typedef struct Canvas{
	unsigned short* addr;
	unsigned int res_hor;
	unsigned int res_ver;
	unsigned int bytes_per_char;
	
	unsigned int text_color;
	unsigned int back_color;
	
	unsigned short cur_x, cur_y;
} Canvas;

typedef enum CanvasColor{
	SCR_COLOR_BLACK = 0x0, 
	SCR_COLOR_BLUE 	= 0x1, 
	SCR_COLOR_GREEN = 0x2, 
	SCR_COLOR_CYAN 	= 0x3, 
	SCR_COLOR_RED 	= 0x4, 
	SCR_COLOR_MAGNETA = 0x5, 
	SCR_COLOR_BROWN = 0x6, 
	SCR_COLOR_WHITE = 0x7 

} CanvasColor;

#define SCR_TABSTOP 4

void sdk_scr_setTextColor(Canvas * info, CanvasColor color);
void sdk_scr_setBackColor(Canvas * info, CanvasColor color);

void sdk_scr_printfXY(Canvas * info, unsigned int x, unsigned int y, const char* fmt, ...);
void sdk_scr_printf(Canvas * info, const char* fmt, ...);

void sdk_scr_clearScreen(Canvas * info, CanvasColor color);

#endif