#ifndef _CPU_DRIVER_H_
#define _CPU_DRIVER_H_

#include "stdint_shared.h"
#include "stddef_shared.h"

typedef enum HW_CPU_FUNC{
	HW_CPU_FUNC_RET_RAM_AMOUNT = 0,
	HW_CPU_FUNC_RET_IRQ_QUEUE_SIZE = 1,
	HW_CPU_FUNC_SET_MMU_TABLE_ADDR = 2
}HW_CPU_FUNC;

typedef enum HW_CPU_INTR{
	HW_CPU_INTR_ABORT = 0,
	HW_CPU_INTR_DIVIDE_BY_ZERO = 1, 
	HW_CPU_INTR_UNDEFINED_INSTRUCTION = 2,
	HW_CPU_INTR_ILLEGAL_INSTRUCTION = 3,
	HW_CPU_INTR_SWI = 4
}HW_CPU_INTR;

void hw_cpu_handleInterrupt(int reason, u32 data0, u32 data1);

unsigned int hw_cpu_retRamAmount(void);
unsigned int hw_cpu_retIrqQueueSize(void);
void hw_cpu_setMmuTableAddress(unsigned int addr);

void hw_cpu_halt()
INLINEASM("\t\
hlt");

uint32_t hw_cpu_nextIRQ(
	__reg("r0") int busid,
	__reg("r4") u32* data0, __reg("r5") u32* data1)
INLINEASM("\t\
nextirq r0\n\t\
; ip will contain 0 if no IRQ was retrieved, (bus<<24)|reason if it was\n\t\
str [r4],r0\n\t\
str [r5],r1\n\t\
mov r0, ip\n\t\
");

#endif