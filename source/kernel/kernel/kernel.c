/*!
*		CROS Kernel Module
*/

//#define DEBUG 1

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

#include "process/process.h"
#include "autorun/autorun.h"

static Process * processKernel;

void krn_start(void);
void krn_waitCycles(int times);
void krn_drawLogo(ScreenInfo * scr_info);

/*!
*	Empty log cback for tlsf
*/
int memCback(const char * fmt, ...){
}

/*!
*	Initialize stdlib memory allocation. 
*/
void krn_initMalloc(void){
	static char test[1024 * 40];
	stdcshared_init(memCback, (void*)&test[1], 1024 * 40);
}


/*!
*	Preparing kernel execution context.
*/
void* krn_init(void){	
	krn_debugLog("==============================");
	krn_initMalloc();
	
	#define APPSTACKSIZE 1024
	processKernel = prc_create("kernel", APPSTACKSIZE, (uint32_t*) &krn_start, USERMODE_SUPERVISOR);
		
	return &processKernel->context;
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

/*!
*	Keyboard interruptions handler. 
* 	Used directly, because kernel executed in privileged mode.
*/
void kybCback(KeyboardEvent event){
	krn_debugLogf("kb = %c", event.key_code);
}

ScreenInfo scr_info;

/*!
*	Kernel entry point. 
*/
void krn_start(void){
	hw_initAll();

	hw_kyb_setCallback(kybCback);
	
	scr_info = hw_scr_screenInfo();		
	krn_drawLogo(&scr_info);
	
	hw_scr_setTextColor(&scr_info, SCR_COLOR_GREEN);
		
	krn_autorun();

	prc_startScheduler();
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
	//hw_cpu_disableIRQ();
	hw_handleInterrupt(krn_currIntrBusAndReason, data0, data1, data2, data3);
	//hw_cpu_enableIRQ();
	
	return processKernel->context;
}

/*!
*	Fatal error handling.
*/
void krn_unexpectedContextSwitch(void){
	krn_debugBSOD("Kernel", "Unexpected context switch.");
	krn_halt();
}