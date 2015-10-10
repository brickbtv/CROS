#include "cpu_driver.h"

#include "kernel/kernel.h"
#include "hwi/hwi.h"

void hw_cpu_handleInterrupt(int reason, u32 data0, u32 data1){
	char mode[3][8] = {"Read   ", "Write  ", "Execute"};
	switch (reason){
		case HW_CPU_INTR_ABORT: 
			krn_debugLogf("INT: CPU Abort. Context Address: %x, Mode: %s", data0, mode[data1]);
			krn_debugBSODf("CPU0 interruption", "Abort. Context address: %x, Mode: %s", data0, mode[data1]);
			break;
		case HW_CPU_INTR_DIVIDE_BY_ZERO:
			krn_debugLogf("INT: CPU Divide by zero.");
			krn_debugBSOD("CPU0 interruption", "Divide by zero");
			break;
		case HW_CPU_INTR_UNDEFINED_INSTRUCTION:
			krn_debugLogf("INT: Undefined instruction.");
			krn_debugBSOD("CPU0 interruption", "Undefined instruction");
			break;
		case HW_CPU_INTR_ILLEGAL_INSTRUCTION:
			krn_debugLogf("INT: Illegal instruction.");
			krn_debugBSOD("CPU0 interruption", "Illegal instruction");
			break;
		case HW_CPU_INTR_SWI:
			krn_debugLogf("INT: Software interruption.");
			break;
	}
}

unsigned int hw_cpu_retRamAmount(void){
	hw_HwiData data;
	
	hwi_call(HW_BUS_CPU, HW_CPU_FUNC_RET_RAM_AMOUNT, &data);
	return data.regs[0];
}

unsigned int hw_cpu_retIrqQueueSize(void){
	hw_HwiData data;
	
	hwi_call(HW_BUS_CPU, HW_CPU_FUNC_RET_IRQ_QUEUE_SIZE, &data);
	return data.regs[0];
}

void hw_cpu_setMmuTableAddress(unsigned int addr){
	hw_HwiData data;
	data.regs[0] = addr;
	
	hwi_call(HW_BUS_CPU, HW_CPU_FUNC_SET_MMU_TABLE_ADDR, &data);
}