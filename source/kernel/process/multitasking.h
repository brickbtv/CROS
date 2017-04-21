#ifndef _MULTITASKING_H_
#define _MULTITASKING_H_

bool isMultitaskingInitialized();
void initMultitasking();

list_node_t * getCurrentProcessNode();
list_node_t * getFocusedProcessNode();

void addProcessToScheduler(void * process);
list_node_t * nextProcess();

list_node_t * firstTaskNode();
list_node_t * lastTaskNode();

void setFocusedProcess(void * prc, void * canvas_address);
list_t * getSchedullerList();
void popFocusedStack();

#endif 