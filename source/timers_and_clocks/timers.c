#include "timers.h"

#include <sdk/clk/clock.h>
#include <stdlib/containers/list.h>
#include <sdk/os/process.h>
#include <sdk/os/debug.h>

#define APP_TIMER_RESERVED 1

list_t * timers_list = NULL;

int getNearestTimer(){
	list_node_t * node;
	list_iterator_t * it = list_iterator_new(timers_list, LIST_HEAD);
	int nearest = 1000000;
	int nearest_id = -1;
	unsigned int current = sdk_clk_timeSinceBoot();
	while(node = list_iterator_next(it)){
		STR_TIMER * timer = (STR_TIMER *)node->val;
		int to_go = timer->ms - (current - timer->last_tick);
		//sdk_debug_logf("TOGO: %d", to_go)
		if (to_go < nearest){
			nearest = to_go;
			nearest_id = timer->timer_number;
		}
	}
	
	return nearest_id;
}


int getNearestTimerDelta(){
	list_node_t * node;
	list_iterator_t * it = list_iterator_new(timers_list, LIST_HEAD);
	int nearest = -1;
	unsigned int current = sdk_clk_timeSinceBoot();
	while(node = list_iterator_next(it)){
		STR_TIMER * timer = (STR_TIMER *)node->val;
		unsigned int to_go = timer->ms - (current - timer->last_tick);
		if (nearest < 0 || to_go < nearest)
			nearest = to_go;		
	}
	
	return nearest;
}

void timers_handleMessage(int type, int reason, int value){
	if (type == SDK_PRC_MESSAGE_CLK){
		int nearest_id = getNearestTimer();
		if (nearest_id >= 0){
			// process timer
			list_node_t * node;
			list_iterator_t * it = list_iterator_new(timers_list, LIST_HEAD);
			while(node = list_iterator_next(it)){
				STR_TIMER * timer = (STR_TIMER*)node->val;
				if (timer->timer_number == nearest_id){
					timer->last_tick = sdk_clk_timeSinceBoot();
					(*timer->cback)(timer->timer_number);
					break;
				}
			}
		}	
	
		// set next timer
		int nearest = getNearestTimerDelta();
		if (nearest >= 0)
			sdk_clk_setCountdownTimer(APP_TIMER_RESERVED, nearest, false);
	}
}

void timers_add_timer(unsigned int timer_number, unsigned int ms, F_TIMER_CBACK timer_cback){
	if (timers_list == NULL)
		timers_list = list_new();
		
	STR_TIMER * str_timer = (STR_TIMER *)calloc(sizeof(STR_TIMER));
	str_timer->timer_number = timer_number;
	str_timer->ms = ms;
	str_timer->cback = timer_cback;
	str_timer->last_tick = sdk_clk_timeSinceBoot();
	//str_timer->owner_process = sdk_prc_
	list_rpush(timers_list, list_node_new(str_timer));
	
	int nearest = getNearestTimerDelta();
	if (nearest > 0){
		sdk_debug_logf("ADD TIMER %d, MS: %d", timer_number, ms);
		sdk_clk_setCountdownTimer(APP_TIMER_RESERVED, nearest, FALSE);
	}
}

void timers_del_timer(unsigned int timer_number){
	if (timers_list == NULL)
		return;
	
	list_node_t * node;
	list_iterator_t * it = list_iterator_new(timers_list, LIST_HEAD);
	while(node = list_iterator_next(it)){
		if (((STR_TIMER*)node->val)->timer_number == timer_number){
			break;
		}
	}
	list_remove(timers_list, node);
}