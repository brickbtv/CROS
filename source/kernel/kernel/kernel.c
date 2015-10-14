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


#include "details/memdetails.h"

static Process processKernel;

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
	processKernel = prc_create("kernel", (uint32_t*)&appStack[APPSTACKSIZE], (uint32_t*) &krn_start, USERMODE_SUPERVISOR);
	
	krn_debugLogf("ad: %s %x", processKernel.name, processKernel.context.gregs[CPU_REG_SP]);
	
	return &processKernel.context;
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

/*!
*	Empty log cback for tlsf
*/
int memCback(const char * fmt, ...){
}

/*!
*	Multiprocessing draft. (Sheduler)
*/
 void krn_ctxswitch(__reg("r0") void* ctx)
 INLINEASM("\t\
 ctxswitch [r0]");

static int np = 0;

static Process p1, p2;

void clkCback(int clk){
	np = (np==0)?1:0;
	
	if (np == 0){
		krn_ctxswitch(&p1.context);
	} else {
		krn_ctxswitch(&p2.context);
	}
}

void p1Ep(void){
	while(true){
		krn_debugLog("p1Ep");
		krn_sleep(500);
	}
}

void p2Ep(void){
	while(true){
		krn_debugLog("p2Ep");
		krn_sleep(500);
	}
}

/*!
*	Kernel entry point. 
*/
void krn_start(void){
	hw_initAll();
	
//	char test[1024 * 4];
//	stdcshared_init(memCback, (void*)&test[1], 1024 * 4);

	hw_kyb_setCallback(kybCback);
	
	ScreenInfo scr_info = hw_scr_screenInfo();		
	krn_drawLogo(&scr_info);
	
	hw_scr_setTextColor(&scr_info, SCR_COLOR_GREEN);

	hw_clk_setTimerCback(KRN_TIMER, clkCback);
	hw_clk_setCountdownTimer(KRN_TIMER, 500, true, true);
	
	static char st1[1024];
	static char st2[1024];
	p1 = prc_create("p1", (uint32_t*)&st1[1024-1], (uint32_t*)p1Ep, USERMODE_SUPERVISOR);
	p2 = prc_create("p2", (uint32_t*)&st2[1024-1], (uint32_t*)p2Ep, USERMODE_SUPERVISOR);

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
	
	return &processKernel.context;
}

/*!
*	Fatal error handling.
*/
void krn_unexpectedContextSwitch(void){
	krn_debugBSOD("Kernel", "Unexpected context switch.");
	krn_halt();
}