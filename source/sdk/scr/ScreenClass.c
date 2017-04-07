#include "ScreenClass.h"

#include <stdarg_shared.h>
#include <string_shared.h>
#include <stdio_shared.h>

void ScreenClass_setTextColor(void * this, CanvasColor color){
	sdk_scr_setTextColor(((ScreenClass*)this)->_canvas, color);
}

void ScreenClass_setBackColor(void * this, CanvasColor color){
	sdk_scr_setBackColor(((ScreenClass*)this)->_canvas, color);
}

void ScreenClass_printfXY(void * this, unsigned int x, unsigned int y, const unsigned char* fmt, ...){
	va_list ap;
	char buf[MAX_STRING_LEN];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	
	sdk_scr_printfXY_no_variadic(((ScreenClass*)this)->_canvas, x, y, buf);
}

void ScreenClass_printf(void * this, const unsigned char* fmt, ...){
	va_list ap;
	char buf[MAX_STRING_LEN];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	
	sdk_scr_printf_no_variadic(((ScreenClass*)this)->_canvas, buf);
}

void ScreenClass_clearScreen(void * this, CanvasColor color){
	sdk_scr_clearScreen(((ScreenClass*)this)->_canvas, color);
}
void ScreenClass_putchar(void * this, unsigned int x, unsigned int y, const unsigned char ch){
	sdk_scr_putchar(((ScreenClass*)this)->_canvas, x, y, ch);
}

int getScreenWidth(void * this){
	return ((ScreenClass*)this)->_canvas->res_hor;
}

int getScreenHeight(void * this){
	return ((ScreenClass*)this)->_canvas->res_ver;
}

Canvas * getCanvas(void * this){
	return ((ScreenClass*)this)->_canvas;
}

ScreenClass * ScreenClass_ctor(ScreenClass * this, Canvas * canvas){
	this->_canvas = canvas;
	
	this->setTextColor = ScreenClass_setTextColor;
	this->setBackColor = ScreenClass_setBackColor;
	this->printfXY = ScreenClass_printfXY;
	this->printf = ScreenClass_printf;
	this->clearScreen = ScreenClass_clearScreen;
	this->putchar = ScreenClass_putchar;
	this->getScreenWidth = getScreenWidth;
	this->getScreenHeight = getScreenHeight;
	
	return this;
}

