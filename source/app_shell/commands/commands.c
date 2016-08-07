#include "commands.h"

#include "filesystem/filesystem.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"

#include "stdlib/details/memdetails.h"

#include <string_shared.h>
#include <stdio_shared.h>

#include "app_texteditor/app_texteditor.h"


void manage_command(Canvas * canvas, char * current_path, const char * input){
	fs_getcwd(current_path, 256);

	if (strcmp(input, "help") == 0){
		sdk_scr_printf(canvas, "wow! such help! much text!\n"
		" 'ls' - show files and folders in current directory\n"
		" 'mkdir NAME' - make new directory\n"
		" 'mkfile NAME' - make new file\n"
		" 'cd NAME' - change directory\n"
		" 'rm NAME' - remove file or directory\n");
	} else if (strncmp(input, "edit ", strlen("edit ")) == 0){
		char * args = malloc(256);
		sprintf(args, "%s/%s", current_path, &input[strlen("edit ")]);
		sdk_debug_logf("edit_file: %s", args);
		sdk_prc_create_process((unsigned int)app_texteditor, args);
	} else if (strcmp(input, "ls") == 0){
		FILEINFO fno;
		int i;
		char *fn;
		FOLDER* dir = fs_opendir(current_path);                       	/* Open the directory */
		if (dir) {
			i = strlen(current_path);
			for (;;) {
				int res = fs_readdir(dir, &fno);                   		/* Read a directory item */
				if (res != FS_OK || fno.fname[0] == 0) break;  			/* Break on error or end of dir */
				
				sdk_scr_printf(canvas, "%c %s\n", fno.ftype, fno.fname);
			}
			fs_closedir(dir);
		}

	} else if (strncmp(input, "mkdir ", strlen("mkdir ")) == 0){
		int res = fs_mkdir(&input[strlen("mkdir ")]);
		if (res != FS_OK)
			sdk_scr_printf(canvas, "Failed to create directory.\n");
	} else if (strncmp(input, "mkfile ", strlen("mkfile ")) == 0){
		FILE * file = fs_open_file(&input[strlen("mkfile ")], 'w');
		if (file){
			fs_close_file(file);
		} else {
			sdk_scr_printf(canvas, "Failed to create file.\n");
		}
	} else if (strncmp(input, "cd ", strlen("cd ")) == 0){
		int res = fs_chdir(&input[strlen("cd ")]);
		if (res != FS_OK)
			sdk_scr_printf(canvas, "Failed to change directory.\n");
		fs_getcwd(current_path, 256);
	} else if (strncmp(input, "rm ", strlen("rm ")) == 0){
		int res = fs_unlink(&input[strlen("rm ")]);
		if (res != FS_OK)
			sdk_scr_printf(canvas, "Failed to remove file or directory.\n");
	} else if (strncmp(input, "cat ", strlen("cat ")) == 0){
		FILE * file = fs_open_file(&input[strlen("cat ")], 'r');
		if (file){
			char buff[1024];
			int rb;
			
			memset(buff, 0, 1024);
			fs_read_file(file, buff, 1024, &rb);
			while(strlen(buff) > 0){
				sdk_scr_printf(canvas, buff);
				memset(buff, 0, 1024);
				if (rb < 1024)
					break;
					
				fs_read_file(file, buff, 1024, &rb);
			}
			sdk_scr_printf(canvas, "\n");
			fs_close_file(file);
		} else {
			sdk_scr_printf(canvas, "Failed to open file.\n");
		}
	}
}