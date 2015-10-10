#ifndef _LOGO_H_
#define _LOGO_H_

#include "hardware\scr\screen_driver.h"

char logo_arr[40*7] = {
0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,								0,   0,   0, '.',   0,    0,    0, '.',    0,    0,
0,0,0,0, 201,205,205,187,0,0,201,205,205,187, 0,0,201,205,205,187,0,0,201,205, 205,187,0,0,0,0, 0,   '\'',0,   0,   0,    0,    0,   0,  ':',    0,
0,0,0,0, 186,0,  0,  0,  0,0,186,0,  0,  186, 0,0,186,  0,  0,186,0,0,186,  0,   0,  0,0,0,0,0, '\'','.', 0,'\'', '.',    0,    0, '.',  ':',    0,
0,0,0,0, 186,0,  0,  0,  0,0,204,205,203,188, 0,0,186,  0,  0,186,0,0,200,205, 205,187,0,0,0,0, 0,   0,   0,   0,   0, '\'',  '.', ':',    0,    0,
0,0,0,0, 186,0,  0,  0,  0,0,186,0,  200,187, 0,0,186,  0,  0,186,0,0,  0,  0,   0,186,0,0,0,0, 0,   0,   0, '.', '.',  ':', '\'','\'',    0,    0,
0,0,0,0, 200,205,205,188,0,0,202,0,  0,  202, 0,0,200,205,205,188,0,0,200,205, 205,188,0,0,0,0, 0,   0, '/','\'',  	0,    0,    0,   0, '\'',  '.',
0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,								0,   0,   0,   0,   0,    0,    0,   0,    0,    0
};









char logo_line_arr[4] = {254, 220, 254, 223};
char logo_line_color_arr[3] = {SCR_COLOR_WHITE, SCR_COLOR_BLUE, SCR_COLOR_RED};

#endif