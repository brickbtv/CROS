/*!
*		CROS Kernel Module
*/

#include "kernel.h"
#include "kernel_debug.h"

#include <string_shared.h>
#include <stdarg_shared.h>
#include <stdio_shared.h>
#include <stddef_shared.h>

#include "hardware/hardware.h"
#include "hardware/cpu/cpu_driver.h"
#include "hardware/clk/clock_driver.h"
#include "hardware/scr/screen_driver.h"
#include "hardware/kyb/keyboard_driver.h"

#include "context/ctx.h"

static Ctx appCtx;

void krn_start(void);
void krn_waitCycles(int times);
void krn_drawLogo(ScreenInfo * scr_info);

/*!
*	Preparing kernel execution context.
*/
void* krn_init(void){	
	krn_debugLog("==============================");
	
	#define APPSTACKSIZE 1024*10
	static char appStack[APPSTACKSIZE];
	memset(&appCtx, 0, sizeof(appCtx));
	appCtx.gregs[CPU_REG_SP] = (u32) &appStack[APPSTACKSIZE];
	appCtx.gregs[CPU_REG_PC] = (u32) &krn_start;
	appCtx.flags = CPU_FLAGSREG_SUPERVISOR;
		
	return &appCtx;
}

/*!
*	Stop OS execution.
*/ 
void krn_halt(void){
	// TODO: disable IRQ before
	while(1){
		hw_cpu_halt();
	}
}

void kybCback(KeyboardEvent event){
	krn_debugLogf("kb = %c", event.key_code);
}

/*!
*	Kernel entry point. 
*/
void krn_start(void){
	hw_initAll();
	
	hw_kyb_setCallback(kybCback);
	
	ScreenInfo scr_info = hw_scr_screenInfo();		
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

/*!
*	Stop kernel execution at specified time.
* 	\param ms Time to sleep in milliseconds
*/
void krn_sleep(unsigned int ms){
	unsigned int start = hw_clk_readTimeSinceBoot();
	unsigned int current = hw_clk_readTimeSinceBoot();
	
	while (current - start < ms){
		if (current < start)
			return;
			
		current = hw_clk_readTimeSinceBoot();
	}
}

/*!
*	Entry point to all execptions.
*	\return basic kernel context
*/
Ctx* krn_handleInterrupt(u32 data0, u32 data1, u32 data2, u32 data3){	
	hw_handleInterrupt(krn_currIntrBusAndReason, data0, data1, data2, data3);
	
	return &appCtx;
}

/*!
*	Fatal error handling.
*/
void krn_unexpectedContextSwitch(void){
	krn_debugBSOD("Kernel", "Unexpected context switch.");
	krn_halt();
}