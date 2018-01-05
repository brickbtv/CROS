#ifndef _assert_shared_h_
#define _assert_shared_h_

#include <sdk/os/process.h>

#ifdef NDEBUG
	#define assert(condition) ((void)0)
#else
	#define assert(condition) if (!(condition)) {sdk_debug_log("Process crashed"); sdk_prc_sleep(1000); sdk_prc_die(); }//__asm("\tdbgbrk\n")
#endif

//#define always_assert(condition)  if (!(condition)) __asm("\tdbgbrk\n")

#endif
