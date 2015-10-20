#include "server.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/nic/network.h"
#include "sdk/kyb/keyboard.h"

#include <string_shared.h>

#include <containers/list.h>
#include <details/memdetails.h>

list_node * root_clients = 0;

typedef struct ClientInfo{
	char name[128];
	int addr;
}ClientInfo;

Canvas * canvas2;

void app_chat_server(){
	canvas2 = (unsigned int*)sdk_prc_getCanvas();
	
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
			sdk_scr_printf(canvas2, "name: %s\n", &msg[10]);
			
			ClientInfo * cl = malloc(sizeof(ClientInfo));
			
			cl->addr = addr;
			strcpy(cl->name, &msg[10]);
			
			if (root_clients == 0){
				root_clients = list_create((void*)cl);
				root_clients->next = 0;
			} else {
				list_node * it = root_clients;
				while (it){
					if (it->next == 0){
						it->next = list_create((void*)cl);
						it->next->next = 0;
						break;
					}
					
					it = it->next;
				}
			}
		}
		
		if ((strlen(msg) > 0) && (strncmp(msg, "> MESSAGE ", 8) == 0)){
			sdk_scr_printf(canvas2, "msg: %s\n", &msg[10]);
			//sender name
			ClientInfo * author;
			list_node * it = root_clients;
			while (it){
				author = (ClientInfo*)it->data;
				if (author->addr == addr){
					break;
				}
				
				it = it->next;
			}
			
			it = root_clients;
			while (it){
				ClientInfo * cl = (ClientInfo*)it->data;
		
				sdk_nic_sendf(cl->addr, "< MESSAGE %s: %s", author->name, &msg[10]);
				
				it = it->next;
			}
		}
	}
}