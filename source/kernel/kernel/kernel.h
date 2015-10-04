#ifndef _KERNEL_H_
#define _KERNEL_H_

void krn_debugLog(const char * msg);
void krn_debugLogf(const char* fmt, ...);

void krn_waitCycles(int times);
#endif