#ifndef _SCREEN_DRIVER_H_
#define _SCREEN_DRIVER_H_

typedef struct ScreenInfo{
	unsigned short* addr;
	unsigned int res_hor;
	unsigned int res_ver;
	unsigned int bytes_per_char;
	
	unsigned int text_color;
	unsigned int back_color;
	
	unsigned short cur_x, cur_y;
} ScreenInfo;

typedef enum ScrColor{
	SCR_COLOR_BLACK = 0x0, 
	SCR_COLOR_BLUE 	= 0x1, 
	SCR_COLOR_GREEN = 0x2, 
	SCR_COLOR_CYAN 	= 0x3, 
	SCR_COLOR_RED 	= 0x4, 
	SCR_COLOR_MAGNETA = 0x5, 
	SCR_COLOR_BROWN = 0x6, 
	SCR_COLOR_WHITE = 0x7 

} ScrColor;

#define SCR_TABSTOP 4 

ScreenInfo hw_scr_init();

void hw_scr_setTextColor(ScreenInfo * info, unsigned int color);
void hw_scr_setBackColor(ScreenInfo * info, unsigned int color);

void hw_scr_putchar(ScreenInfo * info, unsigned int x, unsigned int y, const unsigned char ch);
void hw_scr_printfXY(ScreenInfo * info, unsigned int x, unsigned int y, const char* fmt, ...);
void hw_scr_printf(ScreenInfo * info, const char* fmt, ...);

void hw_scr_clearScreen(ScreenInfo * info, unsigned int color);

#endif