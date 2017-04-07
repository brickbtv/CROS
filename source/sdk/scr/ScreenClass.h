#ifndef _SCREEN_CLASS_H_
#define _SCREEN_CLASS_H_

#include <stdlib/oop.h>
#include "screen.h"

CLASS ScreenClass{
	Canvas * _canvas;
	
	void (* setTextColor)(void * this, CanvasColor color);
	void (* setBackColor)(void * this, CanvasColor color);

	void (* printfXY)(void * this, unsigned int x, unsigned int y, const unsigned char* fmt, ...);
	void (* printf)(void * this, const unsigned char* fmt, ...);

	void (* clearScreen)(void * this, CanvasColor color);
	void (* putchar)(void * this, unsigned int x, unsigned int y, const unsigned char ch);
}ScreenClass;

ScreenClass * ScreenClass_ctor(ScreenClass * this, Canvas * canvas);

#endif