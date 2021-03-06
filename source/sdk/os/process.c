#include "process.h"
#include "sdk/syscall_def.h"
#include "process/process.h"

#include <details/memdetails.h>
#include "debug.h"

int sdk_prc_create_process(unsigned int entry_point, char * name, char * arg_line, Canvas * parent_canvas){
	return app_syscall4(syscall_prc_create_process,(unsigned int)entry_point, (unsigned int)&arg_line[0], (unsigned int)parent_canvas, (unsigned int)&name[0]);
}

unsigned int sdk_prc_sleep(unsigned int ms){
	return app_syscall1(syscall_prc_sleep, ms);
}

void sdk_prc_sleep_until_new_messages(){
	app_syscall0(syscall_prc_sleep_until_new_messages);
}

Canvas * sdk_prc_getCanvas(){
	return (Canvas *)app_syscall0(syscall_prc_getCurrentProcessScreenInfo);
}

void * sdk_prc_getHeapPointer(){
	return (void*)app_syscall0(syscall_prc_getCurrentProcessHeapPointer);
}

bool sdk_prc_haveNewMessage(void){
	return (bool)app_syscall0(syscall_prc_haveNewMessages);
}

void sdk_prc_handleMessage(F_PRC_MSGCBACK cback, void * userdata){
	//PrcMessage * msg = (PrcMessage *)app_syscall0(syscall_prc_getNextMessage);
	unsigned int msg = app_syscall0(syscall_prc_getNextMessage);//msg->type, rs = msg->reason, vl = msg->value;

	int type = (msg   & 0xFF000000) >> 24; 
	int reason = (msg & 0x00FF0000) >> 16;
	int value = msg  & 0x0000FFFF;
			
	cback(type, reason, value, userdata);
}

void sdk_prc_lock(void){
	app_syscall0(syscall_prc_lock);
}

void sdk_prc_unlock(void){
	app_syscall0(syscall_prc_unlock);
}

bool sdk_prc_is_focused(void){
	return app_syscall0(syscall_prc_is_focused);
}

void sdk_prc_die(void){
	app_syscall0(syscall_prc_die);
	while(1){}
	//sdk_prc_sleep(10000);
}

list_t * sdk_prc_get_scheduler_list(void){
	return (list_t *)app_syscall0(syscall_get_scheduler_list);
}

unsigned int sdk_prc_get_total_memory(void){
	return (unsigned int)app_syscall0(syscall_get_total_memory);
}

void sdk_prc_wait_till_process_die(unsigned int pid){
	int alive = 1;
	ProcessDummy * prc;
	while (alive){
		list_t * processes_list = sdk_prc_get_scheduler_list();
		list_node_t * node = processes_list->head;
		alive = 0;
		while (node){
			prc = (ProcessDummy *) node->val;
			if (prc->pid == pid)
				alive = 1;
			node = node->next;
		}
		if (alive)
			sdk_prc_sleep(100);
	}
}