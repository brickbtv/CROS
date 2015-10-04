#include "hardware.h"
#include "kernel/kernel.h"
#include "hwi/hwi.h"
#include <stdio_shared.h>

void hw_initAll(void){
	hw_HwiData data;
	HWERROR err = hwi_call(HWBUS_CPU, HW_CPU_FUNC_RETRAMAMOUNT, &data);
	
	if (err != HWERR_SUCCESS){
		krn_debugLog("DEVICE: cpu0 whi failed"); 
	}
	
	unsigned int ram_in_bytes = data.regs[0];
	krn_debugLogf("DEVICE: RAM amount = %d bytes; (%d KB)", ram_in_bytes, ram_in_bytes / 1024);
		
	// TODO: implement all devices initizlization
}