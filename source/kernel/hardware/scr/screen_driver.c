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
ScreenInfo hw_scr_init(){
	ScreenInfo scr_info;

	hw_HwiData data;
	
	HWERROR err = hwi_call(HWBUS_SCR, HW_SCR_FUNC_SCREENINFO, &data);
	
	scr_info.addr = (void*)data.regs[0];
	scr_info.res_hor = data.regs[1];
	scr_info.res_ver = data.regs[2];
	scr_info.bytes_per_char = data.regs[3];
	
	scr_info.text_color = SCR_COLOR_WHITE;
	scr_info.text_color = SCR_COLOR_BLACK;
	
	return scr_info;
}

void hw_scr_setTextColor(ScreenInfo * info, unsigned int color){
	info->text_color = color << 8;
}

void hw_scr_setBackColor(ScreenInfo * info, unsigned int color){
	info->back_color = color << 12;
}

void hw_scr_putchar(ScreenInfo * info, unsigned int x, unsigned int y, const unsigned char ch){
	short * canvas = info->addr;
	
	*(canvas + y * info->res_hor + x) =  info->back_color | info->text_color | ch;
}

void hw_scr_printf(ScreenInfo * info, unsigned int x, unsigned int y, const char* fmt, ...){
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	char * ch = buf;
	while (*ch){
		hw_scr_putchar(info, x, y, *ch);
		x++;
		ch++;
	}
}
