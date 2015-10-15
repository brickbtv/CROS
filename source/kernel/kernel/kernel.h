#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "kernel_debug.h"
#include "details/memdetails.h"

// reserved. used for multiprocessing
#define KRN_TIMER 0

extern const int krn_currIntrBusAndReason;

void krn_sleep(unsigned int ms);
void krn_halt(void);
#endif