#include "app_chat.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/nic/network.h"

#include <string_shared.h>

void app_chat(void){
	Canvas * canvas = (unsigned int*)sdk_prc_getCanvas();
	sdk_scr_printfXY(canvas, 0, 0, "test new app in user mode!\n");
	
	/*char msg[1024];
	while (1){
		//sdk_scr_printf(canvas, "test\n");
		memset(msg, 0, 1024 * sizeof(char));
		int addr;
		int rec_size = sdk_nic_recv(msg, 1024, &addr);
		if (msg != 0 && strlen(msg) > 0){
			sdk_debug_logf("%s : %d", msg, addr);
		}
		//sdk_prc_sleep(500);
	}*/

	while (1){
		for (int i = 1; i < 60; i++){
			sdk_debug_logf("> %d", i);
			sdk_nic_sendf(i, "test nic msg");
		}
	}
}