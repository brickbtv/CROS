#include "containers/list.h"
#include "details/memdetails.h"
#include "hardware/scr/screen_driver.h"
#include "kernel/kernel_debug.h"

// Processes loop.
list_t * listPrcLoop = NULL;
list_t * stackFocused = NULL;

static list_node_t * currProc = NULL;
static list_node_t * currFocusedProc;

bool isMultitaskingInitialized(){
	return listPrcLoop != NULL;
}

void initMultitasking(){
	// empty scheduler list
	if (listPrcLoop == NULL)				
		listPrcLoop = list_new();
	
	// empty focused processes stack
	if (stackFocused == NULL)				
		stackFocused = list_new();
}

list_node_t * firstTaskNode(){
	return listPrcLoop->head;
}

list_node_t * lastTaskNode(){
	return listPrcLoop->tail;
}


list_node_t * nextProcess(){
	if (currProc->next != NULL){
		currProc = currProc->next;
	} else {
		currProc = firstTaskNode();
	}
	
	return currProc;
}

list_node_t * getCurrentProcessNode(){
	return currProc;
}

list_node_t * getFocusedProcessNode(){
	return currFocusedProc;
}

void addProcessToScheduler(void * process){
	// set up first created process (idle) as current. needed in scheduler.
	if (currProc == NULL){
		currProc = list_rpush(listPrcLoop, list_node_new(process));
	} else {
		list_rpush(listPrcLoop, list_node_new(process));
	}
	
	currFocusedProc = list_rpush(stackFocused, list_node_new(process));
}

list_node_t * removeProcessFromScheduler(list_node_t * node){
	list_node_t * next = node->next;
	list_remove(listPrcLoop, node);
	return next;
}

void setFocusedProcess(void * prc, void * canvas_address){
	list_node_t * node = firstTaskNode();
	while (node && node->val != prc) 
		node = node->next;
	
	currFocusedProc = node;
	hw_scr_mapScreenBuffer(canvas_address);
}

list_t * getSchedullerList(){
	return listPrcLoop;
}

void popFocusedStack(){
	list_rpop(stackFocused);
	currFocusedProc = list_tail(stackFocused);
}