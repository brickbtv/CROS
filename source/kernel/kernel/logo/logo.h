#ifndef _LOGO_H_
#define _LOGO_H_

#include "hardware/scr/screen_driver.h"

#define __ 0
#define rh 201
#define II 186
#define rr 205
#define hr 187
#define jr 200
#define ru 188
#define lr 202
#define lk 203
#define lf 204

#define sm '.'
#define so '\''
#define sl '/'
#define st ':'

char logo_arr[40*7] = {
__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,sm,__,__,__,sm,__,__,
__,__,__,__,rh,rr,rr,hr,__,__,rh,rr,rr,hr,__,__,rh,rr,rr,hr,__,__,rh,rr,rr,hr,__,__,__,__,__,so,__,__,__,__,__,__,st,__,
__,__,__,__,II,__,__,__,__,__,II,__,__,II,__,__,II,__,__,II,__,__,II,__,__,__,__,__,__,__,so,sm,__,so,sm,__,__,sm,st,__,
__,__,__,__,II,__,__,__,__,__,lf,rr,lk,ru,__,__,II,__,__,II,__,__,jr,rr,rr,hr,__,__,__,__,__,__,__,__,__,so,sm,st,__,__,
__,__,__,__,II,__,__,__,__,__,II,__,jr,hr,__,__,II,__,__,II,__,__,__,__,__,II,__,__,__,__,__,__,__,sm,sm,st,so,so,__,__,
__,__,__,__,jr,rr,rr,ru,__,__,lr,__,__,lr,__,__,jr,rr,rr,ru,__,__,jr,rr,rr,ru,__,__,__,__,__,__,sl,so,__,__,__,__,so,sm,
__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__
};

char logo_line_arr[4] = {254, 220, 254, 223};
char logo_line_color_arr[3] = {SCR_COLOR_WHITE, SCR_COLOR_BLUE, SCR_COLOR_RED};

#endif