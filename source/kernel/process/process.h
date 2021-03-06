#ifndef _PROCESS_H_
#define _PROCESS_H_


#include "stdint_shared.h"
#include "string_shared.h"

#include "context/ctx.h"
#include "hardware/scr/screen_driver.h"
#include "hardware/kyb/keyboard_driver.h"

#include "containers/list.h"

#define PRC_CTXSWITCH_RATE_MS 25
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
	
	bool sync_lock;
	bool exist_canvas;
	bool sleep_until_new_messages;
	
	unsigned int firstPage;
	unsigned int numPages;
	unsigned int ds;
	
	unsigned int stackBottom;
	unsigned int stackTop;
	
	int interruptions_count;
	int interruptions_stat[6];
	int interruptions_stat_cpu[25];
	
	bool i_should_die;
	uint32_t entry_point;
	
	// not available for PS utility
	Ctx * context;
	
	uint32_t sleep_start;
	uint32_t sleep_ms;
	
	ScreenInfo * screen;
	
	list_t * list_msgs;
}Process;

typedef struct HeapInfo{
	unsigned int start;
	unsigned int size;
} HeapInfo;

typedef enum PRC_MESSAGE{
	PRC_MESSAGE_KYB = 0,
	PRC_MESSAGE_NIC = 1,
	PRC_MESSAGE_CLK = 2
}PRC_MESSAGE;

typedef struct PrcMessage{
	PRC_MESSAGE type;
	int reason;
	int value;
}PrcMessage;

Process * prc_create(const char * name, uint32_t stackSize, uint32_t heapSize,
						uint32_t * entryPoint, Usermode mode, uint32_t * arg_line, uint32_t * exist_canvas);
void prc_startScheduler(void);
Process * prc_getCurrentProcess(void);
void prc_skipCurrentProc(void);
void sendMessageToAll(PRC_MESSAGE type, int reason, int value);
void sendMessageToFocused(PRC_MESSAGE type, int reason, int value);

bool idleKeyboardEventHandler(KeyboardEvent event);

bool prc_is_focused();
void prc_die();

void prc_initMessagesList();

#endif