#include "autorun.h"

#include "process/process.h"
#include "app_chat/app_chat.h"
#include "app_chat/server.h"
#include "app_test/app_test.h"
#include "app_shell/app_shell.h"

#include <stdint_shared.h>

/*!
*	List of system applications, which starts with the system. Obviously. Srsly.
*/
void krn_autorun(void){
	Process * prc = prc_create("app_shell", 1024*10, 1024*20, (uint32_t*)app_shell, USERMODE_USER, 0, 0);
}