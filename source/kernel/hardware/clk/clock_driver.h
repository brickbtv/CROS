#ifndef _CLOCK_DRIVER_H_
#define _CLOCK_DRIVER_H_

#include <stdint_shared.h>
#include <stddef_shared.h>

typedef enum HW_CLK_FUNC{
	HW_CLK_FUNC_READ_TIME_SINCE_BOOT = 0,
	HW_CLK_FUNC_READ_COUNTDOWN_TIMER = 1, 
	HW_CLK_FUNC_SET_COUNTDOWN_TIMER = 2
}HW_CLK_FUNC;

typedef enum HW_CLK_INTR{
	HW_CLK_INTR_REACHED_COUNTDOWN = 0
}HW_CLK_INTR;

#define HW_CLK_MAXTIMERSNUM 7

void hw_clk_handleInterrupt(int reason, u32 data0, u32 data1);

typedef void (*F_HW_CLK_CBACK)(int);

unsigned int hw_clk_readTimeSinceBoot();
unsigned int hw_clk_readCountdownTimer(unsigned short timer_num);
void hw_clk_setCountdownTimer(unsigned int timer_num, unsigned int ms, bool auto_reset, bool generate_interruption);

void hw_clk_setTimerCback(unsigned int timer_num, F_HW_CLK_CBACK cback);

#endif