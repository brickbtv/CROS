#include "app_elf.h"

#include "sdk/os/process.h"
#include "stdlib/details/memdetails.h"
#include "sdk/scr/ScreenClass.h"
#include <sdk/kyb/keyboard.h>
#include "utils/elf/elf.h"

void msgHndlrElf(int type, int reason, int value, void * userdata){
	if (type == SDK_PRC_MESSAGE_KYB){
		if (reason == KEY_STATE_KEYPRESSED){
			if (value == KEY_RETURN){
				*(int*)userdata = 0;
			}
		}
	}
}

void app_elf(const char* filename){
	Canvas * cv = (Canvas*)sdk_prc_getCanvas();
	
	ScreenClass * screen = malloc(sizeof(ScreenClass));
	screen = ScreenClass_ctor(screen, cv);
	
	elf_dump(screen, filename, 1);
	
	screen->printf(screen, "\n\n\t\tPRESS Enter TO EXIT\n\n");
	
	int noexit = 1;
	while(noexit){		
		while (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHndlrElf, &noexit);
		}
		sdk_prc_sleep_until_new_messages();
	}
	
	sdk_prc_die();
}

void app_elfrun(const char* filename){
	Canvas * cv = (Canvas*)sdk_prc_getCanvas();
	
	ScreenClass * screen = malloc(sizeof(ScreenClass));
	screen = ScreenClass_ctor(screen, cv);	

	if (elf_dump(screen, filename, 0)){
		// error processing
		int noexit = 1;
		while(noexit){		
			while (sdk_prc_haveNewMessage()){
				sdk_prc_handleMessage(msgHndlrElf, &noexit);
			}	
			sdk_prc_sleep_until_new_messages();
		}
	}
	sdk_prc_die();
}
