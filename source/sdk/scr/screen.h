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
	CANVAS_COLOR_BLACK = 0x0, 
	CANVAS_COLOR_BLUE 	= 0x1, 
	CANVAS_COLOR_GREEN = 0x2, 
	CANVAS_COLOR_CYAN 	= 0x3, 
	CANVAS_COLOR_RED 	= 0x4, 
	CANVAS_COLOR_MAGNETA = 0x5, 
	CANVAS_COLOR_BROWN = 0x6, 
	CANVAS_COLOR_WHITE = 0x7 

} CanvasColor;

#define SCR_TABSTOP 4
#define MAX_STRING_LEN 1024

void sdk_scr_setTextColor(Canvas * info, CanvasColor color);
void sdk_scr_setBackColor(Canvas * info, CanvasColor color);

void sdk_scr_printfXY(Canvas * info, unsigned int x, unsigned int y, const unsigned char* fmt, ...);
void sdk_scr_printf(Canvas * info, const unsigned char* fmt, ...);

void sdk_scr_printfXY_no_variadic(Canvas * info, unsigned int x, unsigned int y, const unsigned char* buf);
void sdk_scr_printf_no_variadic(Canvas * info, const unsigned char* buf);

void sdk_scr_clearScreen(Canvas * info, CanvasColor color);
void sdk_scr_clearArea(Canvas * info, CanvasColor color, int x, int y, int width, int height);
void sdk_scr_putchar(Canvas * info, unsigned int x, unsigned int y, const unsigned char ch);

#endif