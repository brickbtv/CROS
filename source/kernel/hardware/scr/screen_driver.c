#include "screen_driver.h"

#include "hardware/hardware.h"
#include "hwi/hwi.h"

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
	
	return scr_info;
}

void hw_scr_putchar(ScreenInfo info, unsigned int x, unsigned int y, unsigned int color, char ch){
	int * canvas = info.addr;
	
	*(canvas + y * info.res_hor + x) =  color | ch;
}