#ifndef _KERNEL_DEBUG_H_
#define _KERNEL_DEBUG_H_

void krn_debugLog(const char * msg);
void krn_debugLogf(const char* fmt, ...);

void krn_debugBSOD(const char * initiator, const char * message);
void krn_debugBSODf(const char * initiator, const char * fmt, ...);

#endif