#include "screen_driver.h"

#include "hardware/hardware.h"
#include "hwi/hwi.h"

#include <string_shared.h>
#include <stdarg_shared.h>
#include <stdio_shared.h>

/*
* 	Screen initialization. 
*	Return: memory address of the screen buffer.
*/

ScreenInfo hw_scr_screenInfo(){
	ScreenInfo scr_info;

	hw_HwiData data;
	
	HWERROR err = hwi_call(HW_BUS_SCR, HW_SCR_FUNC_SCREENINFO, &data);
	
	scr_info.addr = (void*)data.regs[0];
	scr_info.res_hor = data.regs[1];
	scr_info.res_ver = data.regs[2];
	scr_info.bytes_per_char = data.regs[3];
	
	scr_info.text_color = SCR_COLOR_WHITE << 8;
	scr_info.back_color = SCR_COLOR_BLACK << 12; 
	
	scr_info.cur_x = 0;
	scr_info.cur_y = 0;
	
	return scr_info;
}

void hw_scr_mapScreenBuffer(void * addr){
	hw_HwiData data;
	data.regs[0] = (unsigned int)addr;	

	hwi_call(HW_BUS_SCR, HW_SCR_FUNC_MAPSCREENBUFFER, &data);
}

void hw_scr_setTextColor(ScreenInfo * info, unsigned int color){
	info->text_color = color << 8;
}

void hw_scr_setBackColor(ScreenInfo * info, unsigned int color){
	info->back_color = color << 12;
}

void processScroll(ScreenInfo * info){
	// move one line;
	short fullsize = info->res_hor * info->res_ver * info->bytes_per_char;
	short linesize_bytes = info->res_hor;
	short * src = (info->addr + linesize_bytes);
	
	memmove(info->addr, src, fullsize - linesize_bytes * 2);
	memset(info->addr + linesize_bytes * (info->res_ver - 1), 0, info->res_hor);
	
	info->cur_y --;
}

void hw_scr_putchar(ScreenInfo * info, unsigned int x, unsigned int y, const unsigned char ch){
	short * canvas = info->addr;
	
	*(canvas + y * info->res_hor + x) =  info->back_color | info->text_color | ch;
}

void printfXY(ScreenInfo * info, unsigned int x, unsigned int y, const char* buf){
	const char * ch = buf;
	
	info->cur_x = x;
	info->cur_y = y;
	
	while (*ch){
	
		if (info->cur_y >= info->res_ver){
			processScroll(info);
		}
	
		switch(*ch){
			case '\n':
				info->cur_x = 0;
				info->cur_y += 1;
				ch++;	
				
				continue;	
			case '\t':
				info->cur_x += ((info->cur_x % SCR_TABSTOP) == 0) ? SCR_TABSTOP : SCR_TABSTOP - info->cur_x % SCR_TABSTOP;
				ch++;	
			
				continue;
			default: 
				hw_scr_putchar(info, info->cur_x, info->cur_y, *ch);
				info->cur_x += 1;
				ch++;		
		}		
	}
}

void hw_scr_printfXY(ScreenInfo * info, unsigned int x, unsigned int y, const char* fmt, ...){
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	
	printfXY(info, x, y, buf);
}

void hw_scr_printf(ScreenInfo * info, const char* fmt, ...){
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	
	printfXY(info, info->cur_x, info->cur_y, buf);
}

void hw_scr_clearScreen(ScreenInfo * info, unsigned int color){
	unsigned int size = info->res_hor * info->res_ver;
	short space = color << 12 | info->text_color << 8 | ' ';
	
	short * canvas = info->addr;
	
	for (short i = 0; i < size; i++){
		*(canvas + i) = space;
	}
}