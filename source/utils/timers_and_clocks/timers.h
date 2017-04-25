#ifndef _TIMERS_H_
#define _TIMERS_H_

#include <sdk/os/process.h>

typedef void (*F_TIMER_CBACK)(unsigned int timer_number, void * userdata);

typedef struct STR_TIMER{
	unsigned int timer_number;
	unsigned int ms;
	void * owner_process;
	F_TIMER_CBACK cback;
	unsigned int last_tick;
}STR_TIMER;

void timers_handleMessage(int type, int reason, int value, void * userdata);

int timers_add_timer(unsigned int ms, F_TIMER_CBACK timer_cback);
void timers_del_timer(unsigned int timer_number);

#endif