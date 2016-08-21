#include "app_basic.h"
#include <sdk/os/process.h>

#include "basic/ubasic.h"

#include <stdlib/string_shared.h>
#include <stdlib/details/memdetails.h>

#include <filesystem/filesystem.h>

void app_basic(const char* path){
/*	char program[] = 
"10 for i = 0 to 126\n\
20 for j = 0 to 126\n\
30 for k = 0 to 10\n\
40 let a = i * j * k\n\
45 print a, i, j, k\n\
50 next k\n\
60 next j\n\
70 next i\n\
80 stop\n";
*/
	FILE * file = fs_open_file(path, 'r');
	int rb = 0;
	
	char program[2048];
	memset(program, 0, 2048 * sizeof(char));
	fs_read_file(file, program, 2048, &rb);	
	
	fs_close_file(file);
	
	ubasic_init(program);

	do {
		ubasic_run();
	} while(!ubasic_finished());

	sdk_prc_die();
}