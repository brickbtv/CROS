#include "hardware.h"
#include "kernel/kernel.h"
#include "hwi/hwi.h"
#include <stdio_shared.h>

#include "kyb/keyboard_driver.h"
#include "cpu/cpu_driver.h"

void hw_initAll(void){
	/*hw_HwiData data;
	HWERROR err = hwi_call(HWBUS_CPU, HW_CPU_FUNC_RETRAMAMOUNT, &data);
	
	if (err != HWERR_SUCCESS){
		krn_debugLog("DEVICE: cpu0 whi failed"); 
	}
	
	unsigned int ram_in_bytes = data.regs[0];
	krn_debugLogf("DEVICE: RAM amount = %d bytes; (%d KB)", ram_in_bytes, ram_in_bytes / 1024);
		*/
		
	// TODO: implement all devices initizlization
	
	kyb_init();
}

void hw_handleInterrupt(int bus_and_reason, u32 data0, u32 data1, u32 data2, u32 data3){
	uint8_t bus = bus_and_reason >> 24;
	uint32_t reason = bus_and_reason & 0x80FFFFFF;

	switch(bus){
		case HW_BUS_CPU :
			hw_cpu_handleInterrupt(reason, data0, data1);
			break;
		case HW_BUS_CLK: 
			krn_debugLogf("CLK0");
			break;
		case HW_BUS_SCR:
			krn_debugLogf("SCR0");
			break;		
		case HW_BUS_KYB:
			krn_debugLogf("KYB0");
			break;
		case HW_BUS_NIC:
			break;
		case HW_BUS_DKC:
			krn_debugLogf("DKC0");
			break;
		default: 
			krn_debugLogf("DEFAULT0");
	}
}