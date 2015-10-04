/*
*		CROS Kernel Module
*/

#include "kernel.h"
#include "hwi/hwi.h"
#include <string_shared.h>
#include <stdarg_shared.h>
#include <stdio_shared.h>

#include "hardware/hardware.h"
#include "hardware/scr/screen_driver.h"

#define CPU_REG_SP 13
#define CPU_REG_PC 15
#define CPU_FLAGSREG_SUPERVISOR (1<<26)

typedef struct Ctx{
	// Fields that must match the architecture register set
	int gregs[16];
	int flags;
	double fregs[16];
} Ctx;

static Ctx appCtx;

void krn_start(void);
void krn_debugLog(const char * msg);
void krn_waitCycles(int times);

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

void krn_start(void){
	krn_debugLog("CROS: Start");
	
	hw_initAll();
	ScreenInfo scr_info = hw_scr_init();
	krn_debugLogf("SCR: %x, %d, %d, %d", scr_info.addr, scr_info.res_hor, scr_info.res_ver, scr_info.bytes_per_char);
	
	hw_scr_putchar(scr_info, 0, 0, SCR_COLOR_GREEN, 'a');
	
	krn_waitCycles(10000);
	krn_debugLog("CROS: ShutDown");
	krn_waitCycles(10000);
}

void krn_waitCycles(int times){
	for (int i = 0; i < times; i++){
		// do nothing
	}
}

void krn_debugLog(const char * msg){
	hw_HwiData hwi;
	hwi.regs[0] = 0; // Destination id (0 is a debug destination)
	hwi.regs[1] = (unsigned int)msg;
	hwi.regs[2] = strlen(msg) + 1;
	hwi_call(HWBUS_NIC, HW_NIC_FUNC_SEND, &hwi);	
}

void krn_debugLogf(const char* fmt, ...)
{
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	krn_debugLog(buf);
}

Ctx* krn_handleInterrupt(u32 data0, u32 data1, u32 data2, u32 data3){
	return &appCtx;
}