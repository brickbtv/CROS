#ifndef _OS_PROCESS_H_
#define _OS_PROCESS_H_

#include <stddef_shared.h>
#include "containers/list.h"

typedef enum SDK_PRC_MESSAGE{
	SDK_PRC_MESSAGE_KYB = 0,
	SDK_PRC_MESSAGE_NIC = 1,
	SDK_PRC_MESSAGE_CLK = 2
}SDK_PRC_MESSAGE;


typedef struct ProcessDummy{
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
}ProcessDummy;

void sdk_prc_create_process(unsigned int entry_point, char * name, char * arg_line, unsigned int * parent_canvas);

unsigned int sdk_prc_sleep(unsigned int ms);
void sdk_prc_sleep_until_new_messages();
unsigned int * sdk_prc_getCanvas();
void * sdk_prc_getHeapPointer();

typedef void (*F_PRC_MSGCBACK)(int type, int reason, int value, void * userdata);

bool sdk_prc_haveNewMessage(void);
void sdk_prc_handleMessage(F_PRC_MSGCBACK cback, void * userdata);

void sdk_prc_lock(void);
void sdk_prc_unlock(void);

bool sdk_prc_is_focused(void);
void sdk_prc_die(void);

list_t * sdk_prc_get_scheduler_list(void);
#endif