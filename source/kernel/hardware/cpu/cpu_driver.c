#include "cpu_driver.h"

#include "kernel/kernel.h"
#include "hwi/hwi.h"

#include "process/process.h"
#include "syscall/syscall.h"

#define SYSCALL_ID_REGISTER 10

extern F_SYSCALL syscalls_cbacks[];

static char mode[3][8] = {"Read   ", "Write  ", "Execute"};

void hw_cpu_handleInterrupt(int reason, u32 data0, u32 data1){
	Process * prc = prc_getCurrentProcess();

	/*
	*	This check is a hack, because i can't catch why 
	*	process can send SWI and be NULL at the same time.
	*/
	if (prc == NULL){
		krn_debugLogf("WARNING. Process with NULL pointer sent SWI.");
		return;
	}
	
	switch (reason){
		case HW_CPU_INTR_ABORT: 
			krn_debugLogf("INT: CPU Abort. Context Address: %x, Mode: %s. %s", data0, mode[data1], prc->name);
			krn_debugBSODf("CPU0 interruption", "Abort. Context address: %x, Mode: %s", data0, mode[data1]);
			break;
		case HW_CPU_INTR_DIVIDE_BY_ZERO:
			krn_debugLogf("INT: CPU Divide by zero. %s", prc->name);
			krn_debugBSOD("CPU0 interruption", "Divide by zero");
			break;
		case HW_CPU_INTR_UNDEFINED_INSTRUCTION:
			krn_debugLogf("INT: Undefined instruction. %s", prc->name);
			krn_debugBSOD("CPU0 interruption", "Undefined instruction");
			break;
		case HW_CPU_INTR_ILLEGAL_INSTRUCTION:
			krn_debugLogf("INT: Illegal instruction. %s", prc->name);
			krn_debugBSOD("CPU0 interruption", "Illegal instruction");
			break;
		case HW_CPU_INTR_SWI:
			//krn_debugLogf("INT: Software interruption.");
			{
				while (prc->sync_lock){};	// TODO: deffered swi processing				
				
				uint32_t swi_id = prc->context->gregs[SYSCALL_ID_REGISTER];
				syscalls_cbacks[swi_id]();
			}
			
			break;
		default:
			krn_debugBSODf("CPU0 interruption", "Unknown interruption - %d", reason);
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
