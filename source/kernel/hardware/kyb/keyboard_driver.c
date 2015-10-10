#include "keyboard_driver.h"

#include "hardware/hardware.h"
#include "hwi/hwi.h"

void kyb_init(){
	hw_HwiData data;
	//hwi_call(HWBUS_KYB, HW_KYB_FUNC_CLEARBUFFER, &data);
	
	data.regs[0] = 1;
	hwi_call(HW_BUS_KYB, HW_KYB_FUNC_IRQMODE, &data);
}

void kyb_handleInterrupt(u32 data0, u32 data1, u32 data2, u32 data3){
	
}