#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "app_test.h"

void cback();

typedef void (*F_PRC_MSGCBACKs)(void);
void cback_a(F_PRC_MSGCBACKs cb);
int app_test(void){	
	double t = 1234.5678;
	sdk_debug_logf("bardack: %f", t);

    sdk_debug_logf("bardack");
    
	//sdk_debug_logf("bardack: %x %x", *(&t), *(&t+1));
	//sdk_debug_logf("test_text");
	//Canvas * cv = sdk_prc_getCanvas();
	//sdk_scr_clearScreen(cv, 0x3);
	//sdk_prc_sleep(1000);
	cback_a(cback);
	sdk_prc_die();
}

void cback_a(F_PRC_MSGCBACKs cb){
	//int a = 0;
	cb();
}

void cback(void){
	//sdk_debug_logf("cback: 0x%x", cback);
	sdk_debug_logf("TEST");
	//Canvas * cv = sdk_prc_getCanvas();
	//sdk_scr_clearScreen(cv, color);
	//sdk_scr_printfXY(cv, 10, 5, "TEstsetset");
	//sdk_prc_sleep(5000);
}