#include "commands.h"

#include "utils/filesystem/filesystem.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"

#include "stdlib/details/memdetails.h"

#include <string_shared.h>
#include <stdio_shared.h>

#include "app_texteditor/app_texteditor.h"
#include "app_basic/app_basic.h"
#include "app_chat/app_chat.h"
#include "app_chat/server.h"

void manage_command(Canvas * canvas, char * current_path, const char * input){
	fs_getcwd(current_path, 256);

	if (strcmp(input, "help") == 0){
		sdk_scr_printf(canvas, "    CROS help:\n"
		" 'ls' - show files and folders in current directory\n"
		" 'mkdir NAME' - make new directory\n"
		" 'mkfile NAME' - make new file\n"
		" 'cd NAME' - change directory\n"
		" 'rm NAME' - remove file or directory\n"
		" 'cat NAME' - print file content to console\n"
		" 'edit NAME' - simple texteditor\n"
		" 'chat' - simple chat application\n"
		" 'chat_server' - simple chat server for 'chat' app\n");
	} else if (strncmp(input, "edit ", strlen("edit ")) == 0){
		char * args = calloc(256);
		sprintf(args, "%s/%s", current_path, &input[strlen("edit ")]);
	
		FILE * file = fs_open_file(args, 'r');
		if (file){
			fs_close_file(file);
		} else {
			sdk_scr_printf(canvas, "Failed to open file.\n");
			return;
		}
	
		sdk_prc_create_process((unsigned int)app_texteditor, args, 0);
	} else if (strncmp(input, "basic ", strlen("basic ")) == 0){
		char * args = calloc(256);
		sprintf(args, "%s/%s", current_path, &input[strlen("basic ")]);
	
		FILE * file = fs_open_file(args, 'r');
		if (file){
			fs_close_file(file);
		} else {
			sdk_scr_printf(canvas, "Failed to open file.\n");
			return;
		}
		
		sdk_prc_create_process((unsigned int)app_basic, args, canvas);
	} else if (strcmp(input, "chat") == 0){
		sdk_prc_create_process((unsigned int)app_chat, 0, 0);
	} else if (strcmp(input, "chat_server") == 0){
		sdk_prc_create_process((unsigned int)app_chat_server, 0, 0);
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
	} else {
		sdk_scr_printf(canvas, "Unknown command. Type 'help' for commands list.\n");
	}
}