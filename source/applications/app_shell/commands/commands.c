#include "commands.h"

#include "utils/filesystem/filesystem.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/dkc/disk_drive.h"
#include "kernel/hardware/dkc/disk_driver.h"

#include "stdlib/details/memdetails.h"

#include "containers/list.h"

#include <string_shared.h>
#include <stdio_shared.h>
#include <stdlib_shared.h>

#include "app_texteditor/app_texteditor.h"
#include "app_basic/app_basic.h"
#include "app_chat/app_chat.h"
#include "app_chat/server.h"
#include "app_paint/app_paint.h"

#include "sdk/syscall_def.h"

int is_file_exists(char * filename){
	FILE * file = fs_open_file(filename, 'r');
	if (file){
		fs_close_file(file);
		return 1;
	} else {
		return 0;
	}
}

void ls(ScreenClass * screen, char * current_path){
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
}

void ps(ScreenClass * screen){
	ProcessDummy * prc;
	screen->setBackColor(screen, SCR_COLOR_BLUE);
	screen->printf(screen, "PID\tdead\tINTs\tname\t\n");
	screen->setBackColor(screen, SCR_COLOR_BLACK);
	
	list_t * processes_list = sdk_prc_get_scheduler_list();
	list_node_t * node = processes_list->head;
	while (node){
		prc = (ProcessDummy *) node->val;
		screen->printf(screen, "%d\t%d\t\t%d\t\t%s\n", prc->pid, prc->i_should_die, prc->interruptions_count, prc->name);
		node = node->next;
	}
}

void profile(ScreenClass * screen, char * arg){
	

	ProcessDummy * prc;
	int pid = atoi(arg);
	
	list_t * processes_list = sdk_prc_get_scheduler_list();
	list_node_t * node = processes_list->head;
	while (node){
		prc = (ProcessDummy *) node->val;
		if (prc->pid == pid){			
			screen->printf(screen, "%s: Summary %d interruptions\n", prc->name, prc->interruptions_count);
			screen->printf(screen, "    %s - %d\n", "CLK", prc->interruptions_stat[1]);
			screen->printf(screen, "    %s - %d\n", "SCR", prc->interruptions_stat[2]);
			screen->printf(screen, "    %s - %d\n", "KYB", prc->interruptions_stat[3]);
			screen->printf(screen, "    %s - %d\n", "NIC", prc->interruptions_stat[4]);
			screen->printf(screen, "    %s - %d\n", "DKC", prc->interruptions_stat[5]);
			screen->printf(screen, "    %s - %d\n", "CPU", prc->interruptions_stat[0]);			
			for (int i = 0; i < 25; i++)
				if (prc->interruptions_stat_cpu[i] != 0)
					screen->printf(screen, "        %d - %d\t\t%s\n", i, prc->interruptions_stat_cpu[i], SYSCALL_NAMES_DUMMY[i]);
		
			break;
		}
		node = node->next;
	}	
}

void cat(ScreenClass * screen, char * filename){
	FILE * file = fs_open_file(filename, 'r');
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
}

void diskinfo(ScreenClass * screen){
	screen->setBackColor(screen, SCR_COLOR_BLUE);
	screen->printf(screen, "#\tnumSectors\tsectorSize\tstatus\n");
	screen->setBackColor(screen, SCR_COLOR_BLACK);
	
	for (int i = 0; i < 4; i++){
		DiskQuery dq;
		sdk_dkc_get_disk_info(i, &dq);
		
		screen->printf(screen, "%d\t%d\t\t%d\t\t\t%s\t\t\t\n", i, dq.numSectors, dq.sectorSize, dq.errorCode==0?"OK":"NOT CONNECTED");
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
	
		sdk_prc_create_process((unsigned int)app_texteditor, "edit", args, 0);
	} else if (COMMAND_WITH_ARGS("paint ")){
		char * args = calloc(256);
		sprintf(args, "%s/%s", current_path, &input[strlen("paint ")]);
	
		CHECK_FILE_EXIST(args);
	
		sdk_prc_create_process((unsigned int)app_paint, "paint", args, 0);
	} else if (COMMAND_WITH_ARGS("basic ")){
		char * args = calloc(256);
		sprintf(args, "%s/%s", current_path, &input[strlen("basic ")]);
	
		CHECK_FILE_EXIST(args);
		
		sdk_prc_create_process((unsigned int)app_basic, "basic", args, screen->getCanvas());
	} else if (COMMAND("chat")){
		sdk_prc_create_process((unsigned int)app_chat, "chat", 0, 0);
	} else if (COMMAND("chat_server")){
		sdk_prc_create_process((unsigned int)app_chat_server, "chat_server", 0, 0);
	} else if (COMMAND("ls")){
		ls(screen, current_path);
	} else if (COMMAND("ps")){
		ps(screen);
	} else if (COMMAND("diskinfo")){
		diskinfo(screen);
	} else if (COMMAND_WITH_ARGS("profile ")){
		profile(screen, &input[strlen("profile ")]);
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
		cat(screen, &input[strlen("cat ")]);
	} else {
		screen->printf(screen, "Unknown command. Type 'help' for commands list.\n");
	}
}