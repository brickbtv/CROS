#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "kernel_debug.h"

extern const int krn_currIntrBusAndReason;

void krn_waitCycles(int times);
#endif