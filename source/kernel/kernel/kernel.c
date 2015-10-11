/*
*		CROS Kernel Module
*/

#include "kernel.h"
#include "hwi/hwi.h"
#include <string_shared.h>
#include <stdarg_shared.h>
#include <stdio_shared.h>
#include <stddef_shared.h>

#include "hardware/hardware.h"
#include "hardware/scr/screen_driver.h"
#include "hardware/cpu/cpu_driver.h"
#include "hardware/clk/clock_driver.h"
#include "hardware/kyb/keyboard_driver.h"

#include "kernel_debug.h"

#include "logo/logo.h"

#define CPU_REG_SP 13
#define CPU_REG_PC 15
#define CPU_FLAGSREG_SUPERVISOR (1<<26)

typedef struct Ctx{
	int gregs[16];
	int flags;
	double fregs[16];
} Ctx;

static Ctx appCtx;

void krn_start(void);
void krn_waitCycles(int times);
void krn_drawLogo(ScreenInfo * scr_info);

void* krn_init(void){	
	krn_debugLog("==============================");
	krn_debugLog("CROS Kernel: Initialization...");

	#define APPSTACKSIZE 1024*10
	static char appStack[APPSTACKSIZE];
	memset(&appCtx, 0, sizeof(appCtx));
	appCtx.gregs[CPU_REG_SP] = (u32) &appStack[APPSTACKSIZE];
	appCtx.gregs[CPU_REG_PC] = (u32) &krn_start;
	appCtx.flags = CPU_FLAGSREG_SUPERVISOR;
	
	krn_debugLog("CROS Kernel: Done");
	
	return &appCtx;
}

void krn_halt(void){
	while(1){
		hw_cpu_halt();
	}
}

void kybCback(KeyboardEvent event){
	krn_debugLogf("kb = %c", event.key_code);
}

static ScreenInfo scr_info;

void krn_start(void){
	krn_debugLog("CROS: Start");
	
	hw_initAll();
	
	hw_kyb_setCallback(kybCback);
	
	scr_info = hw_scr_screenInfo();
	krn_debugLogf("SCR: 0x%x, %dx%d, %d", scr_info.addr, scr_info.res_hor, scr_info.res_ver, scr_info.bytes_per_char);
		
	krn_drawLogo(&scr_info);
	hw_scr_setTextColor(&scr_info, SCR_COLOR_GREEN);

	for (int i = 0; i < 50; i++){
		hw_scr_printf(&scr_info, "te\tst %d time: %d\n", i, hw_clk_readTimeSinceBoot());
		krn_sleep(500);
	}	
	
	krn_sleep(1000);
	krn_debugLog("CROS: ShutDown");
	krn_halt();
}

void krn_drawLogo(ScreenInfo * scr_info){
	hw_scr_setTextColor(scr_info, SCR_COLOR_GREEN);

	for (int y = 0; y < 7; y++){
		for (int x = 0; x < 40; x++){
			hw_scr_putchar(scr_info, x + 19, y, logo_arr[y*40 + x]);
		}
	}
	
	for (int x = 0; x < 80; x++){
		hw_scr_setTextColor(scr_info, logo_line_color_arr[x%3]);
		hw_scr_putchar(scr_info, x, 7, logo_line_arr[x%4]);
	}
	
	hw_scr_setTextColor(scr_info, SCR_COLOR_GREEN);
	
	scr_info->cur_x = 0;
	scr_info->cur_y = 8;
}


void krn_sleep(unsigned int ms){
	unsigned int start = hw_clk_readTimeSinceBoot();
	unsigned int current = hw_clk_readTimeSinceBoot();
	
	while (current - start < ms){
		if (current < start)
			return;
			
		current = hw_clk_readTimeSinceBoot();
	}
}

Ctx* krn_handleInterrupt(u32 data0, u32 data1, u32 data2, u32 data3){	
	hw_handleInterrupt(krn_currIntrBusAndReason, data0, data1, data2, data3);
	
	return &appCtx;
}