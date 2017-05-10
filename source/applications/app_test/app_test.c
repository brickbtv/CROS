#include "app_test.h"
#include "sdk/os/process.h"

void app_test(void){	

	while(1){
		sdk_prc_sleep_until_new_messages();
	}
}