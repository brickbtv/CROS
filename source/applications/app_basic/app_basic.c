#include "app_basic.h"
#include <sdk/os/process.h>
#include <sdk/scr/screen.h>
#include <sdk/kyb/keyboard.h>

#include "basic/ubasic.h"

#include <stdlib/string_shared.h>
#include <stdlib/details/memdetails.h>

#include <utils/filesystem/filesystem.h>

int exit_basic = 0;

void msgBasicHandlerShell(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_KYB: 
			if (reason == KEY_STATE_KEYTYPED){
				exit_basic = 1;
		}
	}
}

void app_basic(const char* path){
	exit_basic = 0;
	
	FILE * file = fs_open_file(path, 'r');
	int rb = 0;
	
	char program[2048];
	memset(program, 0, 2048 * sizeof(char));
	fs_read_file(file, program, 2048, &rb);	
	
	fs_close_file(file);
	
	ubasic_init(program);

	do {
		ubasic_run();
	} while(!ubasic_finished());

	// wait for user input
	sdk_scr_printf(sdk_prc_getCanvas(), "Program is finished. Press any key to exit...\n");
	while(exit_basic == 0){
		if (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgBasicHandlerShell);
		}
		
		sdk_prc_sleep(500);
	}

	sdk_prc_die();
}