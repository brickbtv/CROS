#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "kernel_debug.h"
#include "details/memdetails.h"
#include "process/process.h"

// reserved. used for multiprocessing
#define KRN_TIMER 0
#define KRN_APP_RESERVED_TIMER 1

#define KRN_HEAP_BASE 1024 * 1000

extern const int krn_currIntrBusAndReason;

int memCback(const char * fmt, ...);
Process * krn_getIdleProcess(void);

bool krn_is_init();
void krn_sleep(unsigned int ms);
void krn_halt(void);
#endif