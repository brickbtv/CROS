#include "autorun.h"

#include "process/process.h"
#include "app_chat/app_chat.h"
#include "app_chat/server.h"

#include <stdint_shared.h>

void krn_autorun(void){
	Process * prc = prc_create("app_chat", 1024*10, (uint32_t*)app_chat, USERMODE_USER);
	//Process * prc = prc_create("app_chat_server", 1024*10, (uint32_t*)app_chat_server, USERMODE_USER);
	hw_scr_mapScreenBuffer((void*)prc->screen->addr);
}