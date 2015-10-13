#include "kernel_debug.h"
#include "kernel.h"

#include <string_shared.h>
#include <stdarg_shared.h>
#include <stdio_shared.h>
#include <stddef_shared.h>

#include "hwi/hwi.h"

#include "hardware/scr/screen_driver.h"
#include "hardware/nic/network_driver.h"

/*!
*	Send message to default debug channel.
*/
void krn_debugLog(const char * msg){
	hw_nic_bufferOutgoingPacket(0, msg, strlen(msg) + 1);
}

/*!
*	Send formatted message to default debug channel
*/
void krn_debugLogf(const char* fmt, ...){
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	krn_debugLog(buf);
}

/*!
*	Show "Blue Screen Of Dead" with reason message.
*/
void krn_debugBSOD(const char * initiator, const char * msg){
	ScreenInfo scr_info = hw_scr_screenInfo();
	hw_scr_clearScreen(&scr_info, SCR_COLOR_BLUE);
	
	hw_scr_setBackColor(&scr_info, SCR_COLOR_BLUE);
	hw_scr_setTextColor(&scr_info, SCR_COLOR_WHITE);
	
	hw_scr_printfXY(&scr_info, 0, 1, " > KERNEL PANIC");
	hw_scr_printfXY(&scr_info, 0, 3, " > INITIATOR: %s", initiator);
	hw_scr_printfXY(&scr_info, 0, 5, " > REASON: %s", msg);
	
	krn_halt();
}

/*!
*	Show formatted "Blue Screen Of Dead" with reason message.
*/

void krn_debugBSODf(const char * initiator, const char * fmt, ...){
	va_list ap;
	char buf[256];
	char* out = &buf[0];
	va_start(ap, fmt);	
	vsprintf(buf, fmt, ap);
	krn_debugBSOD(initiator, buf);
}