#include "gui.h"

#include <stdlib/string_shared.h>

void gui_draw_header(Canvas * cv, const char * title){
	sdk_scr_setBackColor(cv, SCR_COLOR_BLUE);
	
	for(int i = 0; i < cv->res_hor; i++)
		sdk_scr_putchar(cv, i, 0, ' ');

	sdk_scr_printfXY(cv, (cv->res_hor - strlen(title))/2, 0, "%s\n", title);
	
	sdk_scr_setBackColor(cv, SCR_COLOR_BLACK);
}

void gui_draw_bottom(Canvas * cv, const char * bottom){
	sdk_scr_setBackColor(cv, SCR_COLOR_BLUE);
	for(int i = 0; i < cv->res_hor; i++)
		sdk_scr_putchar(cv, i, 24, ' ');
	sdk_scr_printfXY(cv, 0, 24, bottom);
	sdk_scr_setBackColor(cv, SCR_COLOR_BLACK);
}

void gui_draw_area(Canvas * cv, const char * title, unsigned int x, unsigned int y, unsigned int width, unsigned int height){
	unsigned short * addr = cv->addr;
	for (int i = 0; i < width; i++){
		*(addr + cv->res_hor * y + x + i) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | 196;
		*(addr + cv->res_hor * (y + height - 1) + x + i) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | 196;
	}
	
	for (int i = 0; i < height; i++){
		*(addr + cv->res_hor * (y + i) + x) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | 179;
		*(addr + cv->res_hor * (y + i) + x + width - 1) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | 179;
	}
	
	*(addr + cv->res_hor * y + x) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | 218;
	*(addr + cv->res_hor * y + x + width-1) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | 191;
	
	*(addr + cv->res_hor * (y + height - 1) + x) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | 192;
	*(addr + cv->res_hor * (y + height - 1) + x + width - 1) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | 217;
	
	sdk_scr_printfXY(cv, x + 2, y, title);	
	*(addr + cv->res_hor * y + x + 1) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | 180;
	*(addr + cv->res_hor * y + x + 2 + strlen(title)) = SCR_COLOR_BLACK << 12 | SCR_COLOR_WHITE << 8 | 195;
}