#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "kernel_debug.h"

#define KRN_TIMER 0

extern const int krn_currIntrBusAndReason;

void krn_sleep(unsigned int ms);
#endif