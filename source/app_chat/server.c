#include "server.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/nic/network.h"
#include "sdk/kyb/keyboard.h"

#include <string_shared.h>

#include <containers/list.h>
#include <details/memdetails.h>

list_t * list_clients = 0;

typedef struct ClientInfo{
	char name[128];
	int addr;
}ClientInfo;

Canvas * canvas2;

void app_chat_server(){
	canvas2 = (Canvas *)sdk_prc_getCanvas();
	
	sdk_scr_printfXY(canvas2, 0, 0, "Starting server.\n");
	while (1){
		int addr;
		char msg[1024];
		memset(msg, 0, 1024 * sizeof(char));
		int recv_size = sdk_nic_recv(msg, 1024, &addr);
		
		if ((strlen(msg) > 0) && (strcmp(msg, "> SEARCH CR CHAT SERVER") == 0)){
			sdk_scr_printf(canvas2, "New client addr: %d. Reply.\n", addr);
			sdk_nic_send(addr, "< SEARCH RESPONSE", strlen("< SEARCH RESPONSE"));
		}
		
		if ((strlen(msg) > 0) && (strncmp(msg, "> NICKNAME", 8) == 0)){
			sdk_scr_printf(canvas2, "name: %s\n", &msg[11]);
			
			ClientInfo * cl = malloc(sizeof(ClientInfo));
			memset(cl, 0, sizeof(ClientInfo));
			
			cl->addr = addr;
			strncpy(cl->name, &msg[10], strlen(&msg[10]));
			
			if (list_clients == 0){
				list_clients = list_new((void*)cl);
			}
			
			list_rpush(list_clients, list_node_new((void*)cl));			
		}
		
		if ((strlen(msg) > 0) && (strncmp(msg, "> MESSAGE ", 8) == 0)){
			sdk_scr_printf(canvas2, "msg: %s\n", &msg[10]);
			//sender name
			ClientInfo * author = NULL;
			
			list_node_t *node;
			list_iterator_t *it = list_iterator_new(list_clients, LIST_HEAD);
			while ((node = list_iterator_next(it))) {
				author = (ClientInfo*)node->val;
				if (author->addr == addr){
					break;
				}
			}	
			list_iterator_destroy(it);
			
			it = list_iterator_new(list_clients, LIST_HEAD);
			while ((node = list_iterator_next(it))) {
				ClientInfo * cl = (ClientInfo*)node->val;
		
				sdk_nic_sendf(cl->addr, "< MESSAGE %s: %s", author->name, &msg[10]);
			}
			list_iterator_destroy(it);
		}
	}
}