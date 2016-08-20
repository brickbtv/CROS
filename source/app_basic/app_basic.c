#include "app_basic.h"
#include <sdk/os/process.h>

#include "basic/ubasic.h"

#include <stdlib/string_shared.h>
#include <stdlib/details/memdetails.h>


void app_basic(const char* path){
	char program[] = 
"10 print \"dd\"\n\
20 for i = 1 to 10\n\
30 print i\n\
40 next i\n\
50 print \"end\"\n\
60 end\n\
100 print \"subroutine\"\n\
110 return\n";


	char * pr = (char*)calloc(1024);
	strcpy(pr, program);
	ubasic_init(pr);

	do {
		ubasic_run();
	} while(!ubasic_finished());

	sdk_prc_die();
}