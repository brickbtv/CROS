#include "logo.h"

/*!
*	Some useless beauty 
*/
void krn_drawLogo(ScreenInfo * scr_info){
	hw_scr_setTextColor(scr_info, SCR_COLOR_GREEN);

	for (int y = 0; y < 7; y++){
		for (int x = 0; x < 40; x++){
			hw_scr_putchar(scr_info, x + 19, y, logo_arr[y*40 + x]);
		}
	}
	
	for (int x = 0; x < scr_info->res_hor; x++){
		hw_scr_setTextColor(scr_info, logo_line_color_arr[x%3]);
		hw_scr_putchar(scr_info, x, 7, logo_line_arr[x%4]);
	}
	
	hw_scr_setTextColor(scr_info, SCR_COLOR_GREEN);
	
	scr_info->cur_x = 0;
	scr_info->cur_y = 8;
}