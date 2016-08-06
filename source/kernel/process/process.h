#ifndef _PROCESS_H_
#define _PROCESS_H_


#include "stdint_shared.h"
#include "string_shared.h"

#include "context/ctx.h"
#include "hardware/scr/screen_driver.h"

#include "containers/list.h"

#define PRC_CTXSWITCH_RATE_MS 60
#define PRC_MIN_STACKSIZE 64

#define PID_NONE 0
#define PID_KERNEL 1
#define PID_INVALID 255

typedef struct Process{
	char name[50];
	unsigned int pid;
	
	char * arg_line;
	
	char * stack;
	char * heap;
	Ctx * context;
	
	uint32_t sleep_start;
	uint32_t sleep_ms;
	
	ScreenInfo * screen;
	
	list_t * list_msgs;
	
	bool sync_lock;
	
	unsigned int firstPage;
	unsigned int numPages;
	unsigned int ds;
	
	unsigned int stackBottom;
	unsigned int stackTop;
}Process;

typedef struct HeapInfo{
	unsigned int start;
	unsigned int size;
} HeapInfo;

typedef enum PRC_MESSAGE{
	PRC_MESSAGE_KYB = 0,
	PRC_MESSAGE_NIC = 1
}PRC_MESSAGE;

typedef struct PrcMessage{
	PRC_MESSAGE type;
	int reason;
	int value;
}PrcMessage;

Process * prc_create(const char * name, uint32_t stackSize, uint32_t heapSize,
						uint32_t * entryPoint, Usermode mode, uint32_t * arg_line);
void prc_startScheduler(void);
Process * prc_getCurrentProcess(void);
void prc_skipCurrentProc(void);
void sendMessageToAll(PRC_MESSAGE type, int reason, int value);

void prc_initMessagesList();

#endif