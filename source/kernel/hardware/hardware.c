#include "hardware.h"
#include "kernel/kernel.h"
#include "hwi/hwi.h"
#include <stdio_shared.h>

#include "kyb/keyboard_driver.h"
#include "cpu/cpu_driver.h"
#include "clk/clock_driver.h"
#include "nic/network_driver.h"
#include "dkc/disk_driver.h"

void hw_initAll(void){
	// TODO: implement all devices initialization
	
	hw_kyb_init();
	hw_dkc_init();
}

/*!
*	Default handler for all interruptions.
*/
void hw_handleInterrupt(int bus_and_reason, u32 data0, u32 data1, u32 data2, u32 data3){
	uint8_t bus = bus_and_reason >> 24;
	uint32_t reason = bus_and_reason & 0x80FFFFFF;
	
	Process * prc = prc_getCurrentProcess();
	prc->interruptions_count ++;
	prc->interruptions_stat[bus] ++;
	
	switch(bus){
		case HW_BUS_CPU :
			prc->interruptions_stat_cpu[prc->context->gregs[10]] ++;
			hw_cpu_handleInterrupt(reason, data0, data1);
			break;
		case HW_BUS_CLK: 
			hw_clk_handleInterrupt(reason, data0, data1);
			break;
		case HW_BUS_SCR:
			krn_debugBSODf("SCR0 interruption", "Unknown reason - %d", reason);
			break;		
		case HW_BUS_KYB:
			hw_kyb_handleInterrupt(reason);
			break;
		case HW_BUS_NIC:
			hw_nic_handleInterrupt(reason);
			break;
		case HW_BUS_DKC:
			hw_dkc_handleInterrupt(reason, data0, data1);
			break;
		default: 
			krn_debugLogf("DEFAULT0");
			krn_debugBSODf("INTERRUPTION HANDLER", "Bus - %d, Reason - %d", bus, reason);
	}
}