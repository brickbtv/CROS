#ifndef _OS_PROCESS_H_
#define _OS_PROCESS_H_

#include <stddef_shared.h>

typedef enum SDK_PRC_MESSAGE{
	SDK_PRC_MESSAGE_KYB = 0,
	SDK_PRC_MESSAGE_NIC = 1
}SDK_PRC_MESSAGE;

unsigned int sdk_prc_sleep(unsigned int ms);
unsigned int * sdk_prc_getCanvas();
void * sdk_prc_getHeapPointer();

typedef void (*F_PRC_MSGCBACK)(int type, int reason, int value);

bool sdk_prc_haveNewMessage(void);
void sdk_prc_handleMessage(F_PRC_MSGCBACK cback);

void sdk_prc_lock(void);
void sdk_prc_unlock(void);
#endif