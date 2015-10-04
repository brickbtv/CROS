#ifndef _SCREEN_DRIVER_H_
#define _SCREEN_DRIVER_H_

typedef struct ScreenInfo{
	unsigned int* addr;
	unsigned int res_hor;
	unsigned int res_ver;
	unsigned int bytes_per_char;
} ScreenInfo;

#define SCR_COLOR_BLACK 	0x0 << 8
#define SCR_COLOR_BLUE 		0x1 << 8
#define SCR_COLOR_GREEN 	0x2 << 8
#define SCR_COLOR_CYAN 		0x3 << 8
#define SCR_COLOR_RED 		0x4 << 8
#define SCR_COLOR_MAGNETA 	0x5 << 8
#define SCR_COLOR_BROWN 	0x6 << 8
#define SCR_COLOR_WHITE 	0x7 << 8

ScreenInfo hw_scr_init();
void hw_scr_putchar(ScreenInfo info, unsigned int x, unsigned int y, char color, char ch);

#endif