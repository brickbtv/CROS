#include "autorun.h"

#include "process/process.h"
#include "app_chat/app_chat.h"
#include "app_chat/server.h"
#include "app_test/app_test.h"

#include <stdint_shared.h>

/*!
*	List of system applications, which starts with the system. Obviously. Srsly.
*/
void krn_autorun(void){
	//Process * prc = prc_create("app_chat", 1024*5, 1024*10, (uint32_t*)app_chat, USERMODE_USER);
	//Process * prc = prc_create("app_chat_server", 1024*10, 1024*10, (uint32_t*)app_chat_server, USERMODE_USER);
	Process * prc = prc_create("app_test", 1024*20, 1024*20, (uint32_t*)app_test, USERMODE_USER);
	hw_scr_mapScreenBuffer((void*)prc->screen->addr);
}