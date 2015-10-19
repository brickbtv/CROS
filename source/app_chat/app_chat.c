#include "app_chat.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/nic/network.h"
#include "sdk/kyb/keyboard.h"

#include <string_shared.h>

Canvas * canvas;

void msgHandler(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_KYB: 
			if (reason == KEY_STATE_KEYTYPED){
				if (value == 0x02)
					sdk_scr_printf(canvas, "\n");
				else
					sdk_scr_printf(canvas, "%c", value);
			}
			break;
			
		case SDK_PRC_MESSAGE_NIC:
			break;
	}
}

void app_chat(void){
	canvas = (unsigned int*)sdk_prc_getCanvas();
	sdk_scr_printfXY(canvas, 0, 0, "test new app in user mode!\n");
	
	while (1){
		if (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHandler);
		}
	}
}