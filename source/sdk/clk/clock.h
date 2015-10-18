#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <stdint_shared.h>
#include <stddef_shared.h>

unsigned int sdk_clk_timeSinceBoot(void);
unsigned int sdk_clk_readCountdownTimer(uint32_t timerNum);
void sdk_clk_setCountdownTimer(uint32_t timerNum, uint32_t ms, bool autoReset);

#endif