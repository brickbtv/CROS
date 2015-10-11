#include "keyboard_driver.h"

#include "hardware/hardware.h"
#include "hwi/hwi.h"

#include "kernel/kernel.h"

static F_HW_KYB_CBACK hw_kyb_cback = NULL;

void hw_kyb_init(){
	hw_kyb_setIrqMode(1);
}

void hw_kyb_handleInterrupt(unsigned int reason){
	if (reason == HW_KYB_INTR_EVENTAVIALABLE){
		KeyboardEvent event = hw_kyb_getNextChar();
		while (event.event_type != 0){	
			if (hw_kyb_cback != NULL){
				hw_kyb_cback(event);
			}
			
			event = hw_kyb_getNextChar();
		}
	} else {
		krn_debugBSODf("KYB0 interruption", "Unknown reason - %d", reason);
	}
}

void hw_kyb_setCallback(F_HW_KYB_CBACK cback){
	hw_kyb_cback = cback;
}

void hw_kyb_clearBuffer(){
	hw_HwiData data;
	hwi_call(HW_BUS_KYB, HW_KYB_FUNC_CLEARBUFFER, &data);
}

KeyboardEvent hw_kyb_getNextChar(){
	hw_HwiData data;
	hwi_call(HW_BUS_KYB, HW_KYB_FUNC_GETNEXTCHAR, &data);
	
	KeyboardEvent event;
	event.event_type = data.regs[0];
	event.key_code = data.regs[1];
	
	return event;
}

bool hw_kyb_getKeyState(unsigned char key_to_check){
	hw_HwiData data;
	data.regs[0] = key_to_check;
	hwi_call(HW_BUS_KYB, HW_KYB_FUNC_GETKEYSTATE, &data);
	
	return (bool)data.regs[0];
}

void hw_kyb_setIrqMode(bool mode){
	hw_HwiData data;
	data.regs[0] = mode;
	
	hwi_call(HW_BUS_KYB, HW_KYB_FUNC_IRQMODE, &data);
}