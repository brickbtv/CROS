#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "kernel_debug.h"
#include "details/memdetails.h"
#include "process/process.h"

// reserved. used for multiprocessing
#define KRN_TIMER 0

#define KERNEL_STACKSIZE (1024*10)
#define KERNEL_HEAPSIZE (1024*10)

extern const int krn_currIntrBusAndReason;

int memCback(const char * fmt, ...);
Process * krn_getIdleProcess(void);

void krn_sleep(unsigned int ms);
void krn_halt(void);
#endif