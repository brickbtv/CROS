#include "clock_driver.h"

#include "hwi/hwi.h"
#include "kernel/kernel_debug.h"
#include "kernel/kernel.h"

static F_HW_CLK_CBACK hw_clk_cbacks[HW_CLK_MAXTIMERSNUM + 1];

void hw_clk_handleInterrupt(int reason, u32 data0, u32 data1){
	if (reason == HW_CLK_INTR_REACHED_COUNTDOWN){
		for (int i = 0; i < HW_CLK_MAXTIMERSNUM; i++){
			 if (data0 & 1 << i){
				// TODO: need some smater
				if (hw_clk_cbacks[i] != NULL){
					(hw_clk_cbacks[i])(i);
				}
			 }
		}
	} else { 
		krn_debugBSODf("CLK0 interruption", "Unknown interruption - %d", reason);
	}
}
	
unsigned int hw_clk_readTimeSinceBoot(){
	hw_HwiData data;
	hwi_call(HW_BUS_CLK, HW_CLK_FUNC_READ_TIME_SINCE_BOOT, &data);
	
	return data.regs[0];
}

unsigned int hw_clk_readCountdownTimer(unsigned short timer_num){
	if (timer_num > HW_CLK_MAXTIMERSNUM){
		krn_debugLogf("CLK0: incorrect timer number - %d", timer_num);
		return 0;
	}
	
	hw_HwiData data;
	data.regs[0] = timer_num;
	
	hwi_call(HW_BUS_CLK, HW_CLK_FUNC_READ_COUNTDOWN_TIMER, &data);
	
	return data.regs[0];
}

void hw_clk_setCountdownTimer(unsigned int timer_num, unsigned int ms, bool auto_reset, bool generate_interruption){
	if (timer_num > HW_CLK_MAXTIMERSNUM){
		krn_debugLogf("CLK0: incorrect timer number - %d", timer_num);
		return;
	}
	
	hw_HwiData data;
	data.regs[0] = ((unsigned int)auto_reset << 31) | ((unsigned int)generate_interruption << 30) | timer_num;
	data.regs[1] = ms;
	
	hwi_call(HW_BUS_CLK, HW_CLK_FUNC_SET_COUNTDOWN_TIMER, &data);
}



void hw_clk_setTimerCback(unsigned int timer_num, F_HW_CLK_CBACK cback){
	if (timer_num > HW_CLK_MAXTIMERSNUM){
		krn_debugLogf("CLK0: incorrect timer number - %d", timer_num);
		return;
	}
	
	hw_clk_cbacks[timer_num] = cback;
}