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
#include "app_paint/app_paint.h"

int is_file_exists(char * filename){
	FILE * file = fs_open_file(filename, 'r');
	if (file){
		fs_close_file(file);
		return 1;
	} else {
		return 0;
	}
}

void manage_command(ScreenClass * screen, char * current_path, const char * input){
	#define COMMAND(cmd) strcmp(input, cmd) == 0
	#define COMMAND_WITH_ARGS(cmd) strncmp(input, cmd, strlen(cmd)) == 0
	#define CHECK_FILE_EXIST(filename) if (!is_file_exists(filename){\
			screen->printf(screen, "Failed to open file.\n");\
			return;\
		}\

	fs_getcwd(current_path, 256);

	if (COMMAND("help")){
		screen->printf(screen, "    CROS help:\n"
		" 'ls' - show files and folders in current directory\n"
		" 'mkdir NAME' - make new directory\n"
		" 'mkfile NAME' - make new file\n"
		" 'cd NAME' - change directory\n"
		" 'rm NAME' - remove file or directory\n"
		" 'cat NAME' - print file content to console\n"
		" 'edit NAME' - simple texteditor\n"
		" 'chat' - simple chat application\n"
		" 'chat_server' - simple chat server for 'chat' app\n");
	} else if (COMMAND_WITH_ARGS("edit ")){
		char * args = calloc(256);
		sprintf(args, "%s/%s", current_path, &input[strlen("edit ")]);
	
		CHECK_FILE_EXIST(args);
	
		sdk_prc_create_process((unsigned int)app_texteditor, args, 0);
	} else if (COMMAND_WITH_ARGS("paint ")){
		char * args = calloc(256);
		sprintf(args, "%s/%s", current_path, &input[strlen("paint ")]);
	
		CHECK_FILE_EXIST(args);
	
		sdk_prc_create_process((unsigned int)app_paint, args, 0);
	} else if (COMMAND_WITH_ARGS("basic ")){
		char * args = calloc(256);
		sprintf(args, "%s/%s", current_path, &input[strlen("basic ")]);
	
		CHECK_FILE_EXIST(args);
		
		sdk_prc_create_process((unsigned int)app_basic, args, screen->getCanvas());
	} else if (COMMAND("chat")){
		sdk_prc_create_process((unsigned int)app_chat, 0, 0);
	} else if (COMMAND("chat_server")){
		sdk_prc_create_process((unsigned int)app_chat_server, 0, 0);
	} else if (COMMAND("ls")){
		FILEINFO fno;
		int i;
		char *fn;
		FOLDER* dir = fs_opendir(current_path);                       	/* Open the directory */
		if (dir) {
			i = strlen(current_path);
			for (;;) {
				int res = fs_readdir(dir, &fno);                   		/* Read a directory item */
				if (res != FS_OK || fno.fname[0] == 0) break;  			/* Break on error or end of dir */
				
				if (fno.ftype == 'f')
					screen->printf(screen, "%c %s \t\t\t%db\n", fno.ftype, fno.fname, fno.fsize);
				else 
					screen->printf(screen, "%c %s\n", fno.ftype, fno.fname);
			}
			fs_closedir(dir);
		}

	} else if (COMMAND_WITH_ARGS("mkdir ")){
		int res = fs_mkdir(&input[strlen("mkdir ")]);
		if (res != FS_OK)
			screen->printf(screen, "Failed to create directory.\n");
	} else if (COMMAND_WITH_ARGS("mkfile ")){
		FILE * file = fs_open_file(&input[strlen("mkfile ")], 'w');
		if (file){
			fs_close_file(file);
		} else {
			screen->printf(screen, "Failed to create file.\n");
		}
	} else if (COMMAND_WITH_ARGS("cd ")){
		int res = fs_chdir(&input[strlen("cd ")]);
		if (res != FS_OK)
			screen->printf(screen, "Failed to change directory.\n");
		fs_getcwd(current_path, 256);
	} else if (COMMAND_WITH_ARGS("rm ")){
		int res = fs_unlink(&input[strlen("rm ")]);
		if (res != FS_OK)
			screen->printf(screen, "Failed to remove file or directory.\n");
	} else if (COMMAND_WITH_ARGS("cat ")){
		FILE * file = fs_open_file(&input[strlen("cat ")], 'r');
		if (file){
			char buff[1024];
			int rb;
			
			memset(buff, 0, 1024);
			fs_read_file(file, buff, 1024, &rb);
			while(strlen(buff) > 0){
				screen->printf(screen, buff);
				memset(buff, 0, 1024);
				if (rb < 1024)
					break;
					
				fs_read_file(file, buff, 1024, &rb);
			}
			screen->printf(screen, "\n");
			fs_close_file(file);
		} else {
			screen->printf(screen, "Failed to open file.\n");
		}
	} else {
		screen->printf(screen, "Unknown command. Type 'help' for commands list.\n");
	}
}