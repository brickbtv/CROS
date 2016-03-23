#include "screen.h"

#include <string_shared.h>
#include <stdarg_shared.h>
#include <stdio_shared.h>

void sdk_scr_setTextColor(Canvas * info, CanvasColor color){
	info->text_color = color << 8;
}

void sdk_scr_setBackColor(Canvas * info, CanvasColor color){
	info->back_color = color << 12;
}

void putchar(Canvas * info, unsigned int x, unsigned int y, const unsigned char ch){
	short * canvas = info->addr;
	
	*(canvas + y * info->res_hor + x) =  info->back_color | info->text_color | ch;
}

void sdk_scr_putchar(Canvas * info, unsigned int x, unsigned int y, const unsigned char ch){
	putchar(info, x, y, ch);
}

void processScroll(Canvas * info){
	// move one line;
	short fullsize = info->res_hor * info->res_ver * info->bytes_per_char;
	short linesize_bytes = info->res_hor;
	short * src = (info->addr + linesize_bytes);
	
	memmove(info->addr, src, fullsize - linesize_bytes * 2);
	memset(info->addr + linesize_bytes * (info->res_ver - 1), 0, info->res_hor);
	
	info->cur_y --;
}

void printfXY(Canvas * info, unsigned int x, unsigned int y, const unsigned char* buf){
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
				putchar(info, info->cur_x, info->cur_y, *ch);
				info->cur_x += 1;
				ch++;		
		}		
	}
}

void sdk_scr_printfXY(Canvas * info, unsigned int x, unsigned int y, const unsigned char* fmt, ...){
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	
	printfXY(info, x, y, buf);
}

void sdk_scr_printf(Canvas * info, const unsigned char* fmt, ...){
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
		
	printfXY(info, info->cur_x, info->cur_y, buf);
}

void sdk_scr_clearScreen(Canvas * info, CanvasColor color){
	unsigned int size = info->res_hor * info->res_ver;
	short space = color << 12 | info->text_color << 8 | ' ';
	
	short * canvas = info->addr;
	
	for (short i = 0; i < size; i++){
		*(canvas + i) = space;
	}
}
