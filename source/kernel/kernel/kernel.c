/*!
*		CROS Kernel Module
*/

//#define DEBUG 1

#include "boot/boot.h"

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

#include "autorun/autorun.h"

static Process * processKernel;

void krn_start(void);
void krn_waitCycles(int times);
void krn_drawLogo(ScreenInfo * scr_info);

/*!
*	Empty log cback for tlsf
*/
int memCback(const char * fmt, ...){
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	
	krn_debugLog(buf);
	
	return 1;
}

int align(unsigned int pointer){
	return 4 - pointer % 4;
}

/*!
*	Initialize stdlib memory allocation. 
*/
void krn_initMalloc(void){
	static char kernel_hp[1024 * 100+10];
	unsigned int align_offset = (unsigned int)&kernel_hp[0];
	int a = align(align_offset);
	
	stdcshared_init(memCback, (void*)&kernel_hp[a], 1024 * 300);
}


/*!
*	Preparing kernel execution context.
*/
bool krn_inited = false;

void* krn_init(void){	
	krn_debugLog("==============================");
	krn_initMalloc();
	
	#define APPSTACKSIZE 1024*10
	processKernel = prc_create("kernel", APPSTACKSIZE, 1024*20, (uint32_t*) &krn_start, USERMODE_SUPERVISOR, 0);
	
	krn_autorun();
		
	krn_inited = true;
	return &processKernel->context;
}

/*!
*	@internal @private
*	used for kernel malloc impl
*/
bool krn_is_init(){
	return krn_inited;
}

/*! 
*	Returns main process
*/
Process * krn_getIdleProcess(void){
	return processKernel;
}

/*!
*	Stop OS execution until any interruption
*/ 
void krn_halt(void){
	hw_cpu_halt();
}

/*!
*	Keyboard interruptions handler. 
* 	Used directly, because kernel executed in privileged mode.
*/
void kybCback(KeyboardEvent event){
	sendMessageToAll(PRC_MESSAGE_KYB, event.event_type, event.key_code);
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
		
	hw_scr_printfXY(&scr_info, 0, 0, "%d %d \n%d %d \n%d %d\n\n",
						processInfo.readOnlyAddr, 
						processInfo.readOnlySize,
						processInfo.readWriteAddr,
						processInfo.readWriteSize,
						processInfo.sharedReadWriteAddr,
						processInfo.sharedReadWriteSize
						);
						
	hw_scr_printf(&scr_info, "%d", hw_cpu_retRamAmount());
					
	/*
		messages list should be initializated inside Idle process,
		because it uses malloc with it's own dynamic memory pool
	*/
	prc_initMessagesList();
					
	//krn_autorun();

	prc_startScheduler();
}

/*!
*	Stop kernel execution at specified time.
* 	\param ms Time to sleep in milliseconds
*/
void krn_sleep(unsigned int ms){
	unsigned int start = hw_clk_readTimeSinceBoot();
	unsigned int current = hw_clk_readTimeSinceBoot();
	
	while (current - start <= ms){
		if (current < start)
			return;
			
		current = hw_clk_readTimeSinceBoot();
	}
}

/*!
*	Entry point to all interruptions.
*	\return basic kernel context
*/

#define NO_INTERRUPT -1
extern const int krn_prevIntrBusAndReason;

Ctx* krn_handleInterrupt(u32 data0, u32 data1, u32 data2, u32 data3){
	if (krn_getIdleProcess() != prc_getCurrentProcess())
		if (krn_getIdleProcess()->sync_lock)
			return prc_getCurrentProcess()->context;
	//else 
	//	krn_getIdleProcess()->sync_lock = true;
		
	// Check for double faults (kernel crashes)
	// They are detecter by checking if we were serving an interrupt before
	if (krn_prevIntrBusAndReason!=NO_INTERRUPT && (krn_currIntrBusAndReason >> 24) != 0)
	{
		krn_debugBSODf("Interruptions handler",	"DOUBLE FAULT: PREVIOUS %d %d, DATA 0x%X,0x%X,0x%X,0x%X\n  CURRENT: %d %d",
			(krn_prevIntrBusAndReason >> 24), (krn_prevIntrBusAndReason & 0x80FFFFFF), data0, data1, data2, data3,
			(krn_currIntrBusAndReason >> 24), (krn_currIntrBusAndReason & 0x80FFFFFF));
		// note: panic never returns, so we never get here
	}
	
	int busAndReason = krn_currIntrBusAndReason;
	do {
	
		hw_handleInterrupt(busAndReason, data0, data1, data2, data3);
		busAndReason = hw_cpu_nextIRQ(-1, &data0, &data1);
	} while (busAndReason);
	
	//if (krn_getIdleProcess() == prc_getCurrentProcess())
	//	krn_getIdleProcess()->sync_lock = false;
	
	return prc_getCurrentProcess()->context;
}

/*!
*	Fatal error handling.
*/
void krn_unexpectedContextSwitch(void){
	krn_debugBSOD("Kernel", "Unexpected context switch.");
	krn_halt();
}