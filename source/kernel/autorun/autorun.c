#include "autorun.h"

#include "process/process.h"
#include "app_chat/app_chat.h"

#include <stdint_shared.h>

void krn_autorun(void){
	Process * prc = prc_create("app_chat", 1024, (uint32_t*)app_chat, USERMODE_USER);
	hw_scr_mapScreenBuffer((void*)prc->screen->addr);
}