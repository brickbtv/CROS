#ifndef _KEYBOARD_DRIVER_H_
#define _KEYBOARD_DRIVER_H_

#include "stddef_shared.h"

typedef enum HW_KYB_FUNC{
	HW_KYB_FUNC_CLEARBUFFER = 0,
	HW_KYB_FUNC_GETNEXTCHAR = 1,
	HW_KYB_FUNC_GETKEYSTATE = 2,
	HW_KYB_FUNC_IRQMODE = 3
}HW_KYB_FUNC;

typedef enum HW_KYB_INTR{
	HW_KYB_INTR_EVENTAVIALABLE
}HW_KYB_INTR;

typedef enum HW_KYB_EVENTTYPE{
	HW_KYB_EVENTTYPE_EMPTYBUFFER = 0,
	HW_KYB_EVENTTYPE_KEYPRESSED = 1,
	HW_KYB_EVENTTYPE_KEYRELEASED = 2,
	HW_KYB_EVENTTYPE_KEYTYPED = 3
}HW_KYB_EVENTTYPE;

typedef struct KeyboardEvent{
	HW_KYB_EVENTTYPE event_type;
	unsigned char key_code;
}KeyboardEvent;

void hw_kyb_init(void);
void hw_kyb_handleInterrupt(unsigned int reason);

typedef void (*F_HW_KYB_CBACK)(KeyboardEvent);
void hw_kyb_setCallback(F_HW_KYB_CBACK cback);

void hw_kyb_clearBuffer();
KeyboardEvent hw_kyb_getNextChar();
bool hw_kyb_getKeyState(unsigned char key_to_check);
void hw_kyb_setIrqMode(bool mode);

#endif