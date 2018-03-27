#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/kyb/keyboard.h"
#include "sdk/nic/network.h"
#include "app_test.h"
#include "stdlib/string_shared.h"


#include "utils/networking/networking.h"

int c = 0;

void msgHndlrTest(int type, int reason, int value, void * userdata){
	if (type == SDK_PRC_MESSAGE_NIC){
		c++;		
		
		unsigned char buffer[1024];
		memset(buffer, 0, 1024 * sizeof(unsigned char));
		int addr = 0;
		int rb = sdk_nic_recv(&buffer[0], 1024, &addr);
		
		if (addr == 0)
			return;
		
		if (c % 100 == 0)
			sdk_debug_logf("$$$ %d", c);
			
		STR_NET_SOCKET socket_srv = net_client(125, sdk_nic_getAddress(), NET_PROTO_FILETRANSFER);
		unsigned char text[1024];
		net_recv(&socket_srv, buffer, text, 1024);
		
		sdk_debug_logf("Payload: %s", text);
		
		//STR_NET_SOCKET socket_cl = net_client(125, sdk_nic_getAddress(), NET_PROTO_FILETRANSFER);
		//net_send(&socket_cl, "TEST", 4);
	}

	if (type == SDK_PRC_MESSAGE_KYB){
		if (reason == KEY_STATE_KEYPRESSED){
			if (value == KEY_RETURN){
				*(int*)userdata = 0;
			}
		}
	}
}

int app_test(void){		
	int noexit = 1;

	//sdk_nic_send(sdk_nic_getAddress(), "TEST", 4);
	
	STR_NET_SOCKET socket_cl = net_client(125, sdk_nic_getAddress(), NET_PROTO_FILETRANSFER);
	net_send(&socket_cl, "TEST", 4);
	

	while (noexit)
		while (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHndlrTest, &noexit);
		}	
		sdk_prc_sleep_until_new_messages();
	
	sdk_prc_die();
}

