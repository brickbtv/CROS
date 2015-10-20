#include "app_chat.h"

#include "sdk/scr/screen.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/nic/network.h"
#include "sdk/kyb/keyboard.h"

#include <string_shared.h>

Canvas * canvas;

static int line = 1;

static char input[1024];
static int symb = 0;

static int server_addr = 0;

static int state = 0;	//0 - connection; 1 - name request ; 2 - chat;

void msgHandler(int type, int reason, int value){
	switch (type){
		case SDK_PRC_MESSAGE_KYB: 
			sdk_debug_logf("ki: %c", value);
			if (reason == KEY_STATE_KEYTYPED){
				if (value == 0x01){
					if (--symb <= 0){
						symb = 0;
					}
					input[symb] = 0;
				} else if (value == 0x02){
					switch (state){
						case 1:
							state = 2;
							// send name
							sdk_nic_sendf(server_addr, "> NICKNAME %s", input);
							draw_fake_gui();
							
							break;
						case 2:
							sdk_nic_sendf(server_addr, "> MESSAGE %s", input);
							sdk_scr_printfXY(canvas, 3, canvas->res_ver - 2, "                               ");
							break;
					}
					
					memset(input, 0, 1024 * sizeof(char));
					symb = 0;
				} else {
					input[symb++] = value;
				}
				
				
				switch (state){
					case 1:
						sdk_scr_printfXY(canvas, 0, 2, "%s ", input);
						break;
					case 2:
						sdk_scr_setBackColor(canvas, SCR_COLOR_BLACK);
						sdk_scr_printfXY(canvas, 3, canvas->res_ver - 2, "%s ", input);
						break;
				}
			}
			break;
			
		case SDK_PRC_MESSAGE_NIC:
			break;
	}
}

void app_chat(void){
	canvas = (unsigned int*)sdk_prc_getCanvas();
			
	memset(input, 0, 1024 * sizeof(char));
		
	while (1){
		switch (state){
			case 0:
				sdk_scr_printfXY(canvas, 0, 0, "Search server... ")
				server_addr = findServer();
				state = 1;
				sdk_scr_printf(canvas, "done.\nEnter your name: \n");
				break;
			case 1:
				
				
				break;
			case 2:
				{
					char msg[1024];
					int addr;
					
					// new message received?
					memset(msg, 0, 1024 * sizeof(char));
					int recvs = sdk_nic_recv(msg, 1024, &addr);
					if ((strlen(msg) > 0) && (strncmp("< MESSAGE", msg, 6) == 0)){
						sdk_scr_printfXY(canvas, 2, line, " > %s", &msg[10]);
						line++;							
					}
						
				}
				break;
		}
	
		if (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHandler);
		}
	}
}

int findServer(){
	while (1){
		char msg[1024];
		int addr;
		
		// send broadcast
		for (int i = 1; i < 1024; i++){
			char req[40] = "> SEARCH CR CHAT SERVER";
			sdk_nic_send(i, req, strlen(req));
			
			memset(msg, 0, 1024 * sizeof(char));
			int recvs = sdk_nic_recv(msg, 1024, &addr);
			if ((strlen(msg) > 0) && (strcmp("< SEARCH RESPONSE", msg) == 0)){
				return addr;
			} 
		}		
	}
}



void draw_fake_gui(){
	sdk_scr_clearScreen(canvas, SCR_COLOR_BLACK);
	sdk_scr_setTextColor(canvas, SCR_COLOR_BROWN);
	
	for (int i = 0; i < canvas->res_hor; i++){
		sdk_scr_putchar(canvas, i, 0, 219);
		sdk_scr_putchar(canvas, i, canvas->res_ver - 1, 219);
		if (i < canvas->res_hor - 20)
			sdk_scr_putchar(canvas, i, canvas->res_ver - 3, 219);
	}
		
	for (int i = 0; i < canvas->res_ver; i++){
		sdk_scr_putchar(canvas, 0, i, 219);
		sdk_scr_putchar(canvas, canvas->res_hor - 1, i, 219);
		sdk_scr_putchar(canvas, canvas->res_hor - 20, i, 219);
	}
	
	sdk_scr_setTextColor(canvas, SCR_COLOR_WHITE);
	sdk_scr_setBackColor(canvas, SCR_COLOR_BROWN);
	sdk_scr_printfXY(canvas, 30, 0, "CR Chat:");
	sdk_scr_printfXY(canvas, 65, 0, "Users:");
	sdk_scr_printfXY(canvas, 2, canvas->res_ver-3, "Input message:");
}