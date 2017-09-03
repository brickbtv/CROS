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
#include "app_disasm/app_disasm.h"
#include "app_memprof/app_memprof.h"

#include "sdk/syscall_def.h"
#include "../mkfs/mkfs.h"

int is_file_exists(const char * filename){
	FILE * file = fs_open_file(filename, 'r');
	if (file){
		fs_close_file(file);
		return 1;
	} else {
		return 0;
	}
}

void ls(ScreenClass * screen, const char * current_path){
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
 	screen->setBackColor(screen, CANVAS_COLOR_BLUE);
	screen->printf(screen, "PID\tdead\tINTs\tname\t\n");
	screen->setBackColor(screen, CANVAS_COLOR_BLACK);
	
	list_t * processes_list = sdk_prc_get_scheduler_list();
	list_node_t * node = processes_list->head;
	while (node){
		prc = (ProcessDummy *) node->val;
		screen->printf(screen, "%d\t%d\t\t%d\t\t%s\n", prc->pid, prc->i_should_die, prc->interruptions_count, prc->name);
		node = node->next;
	}
}

void profile(ScreenClass * screen, const char * arg){
	ProcessDummy * prc;
	int pid = atoi(arg);
	
	list_t * processes_list = sdk_prc_get_scheduler_list();
	list_node_t * node = processes_list->head;
	while (node){
		prc = (ProcessDummy *) node->val;
		if (prc->pid == pid){			
			screen->printf(screen, "%s: Summary %d interruptions\n", prc->name, prc->interruptions_count);
			/*screen->printf(screen, "    %s - %d\n", "CLK", prc->interruptions_stat[1]);
			screen->printf(screen, "    %s - %d\n", "SCR", prc->interruptions_stat[2]);
			screen->printf(screen, "    %s - %d\n", "KYB", prc->interruptions_stat[3]);
			screen->printf(screen, "    %s - %d\n", "NIC", prc->interruptions_stat[4]);
			screen->printf(screen, "    %s - %d\n", "DKC", prc->interruptions_stat[5]);*/
			screen->printf(screen, "    %s - %d\n", "CPU", prc->interruptions_stat[0]);
			for (int i = 0; i < 25; i++)
				if (prc->interruptions_stat_cpu[i] != 0)
					screen->printf(screen, "        %d - %d\t\t%s\n", i, prc->interruptions_stat_cpu[i], SYSCALL_NAMES_DUMMY[i]);
		
			break;
		}
		node = node->next;
	}	
}

void cat(ScreenClass * screen, const char * filename){
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
	screen->setBackColor(screen, CANVAS_COLOR_BLUE);
	screen->printf(screen, "#\tFatFS\tnumSectors\tsectorSize\tstatus\n");
	screen->setBackColor(screen, CANVAS_COLOR_BLACK);
	
	for (int i = 0; i < 4; i++){
		DiskQuery dq;
		sdk_dkc_get_disk_info(i, &dq);
		int res = fs_mount_drive(i);
		
		char status[4];
		if (res == FS_NO_FILESYSTEM)
			strcpy(status, "No ");
		else if (res == FS_OK)
			strcpy(status, "Yes");
		else
			strcpy(status, "---");
		status[3] = 0;
		
		if (dq.errorCode == 0)
			screen->printf(screen, "%d\t%s\t\t%d\t\t%d\t\t\t%s\t\t\t\n", i, status, dq.numSectors, dq.sectorSize, "OK");
		else 
			screen->printf(screen, "%d\t%s\t\t%s\t\t\t%s\t\t\t%s\t\t\t\n", i, status, "---", "---", "NOT CONNECTED");
	}
}

void cd (ScreenClass * screen, const char * current_path, const char * path){
	int res = fs_chdrive(path);
	if (res != FS_OK){
		screen->printf(screen, "Failed to change drive.\n");
		return;
	}
	res = fs_chdir(path);
	if (res != FS_OK){
		screen->printf(screen, "Failed to change directory.\n");
		return;
	}
	fs_getcwd(current_path, 256);
}

void cp(ScreenClass * screen, const char * args){
	int pos = find(args, ' ', 0);
	if (pos == -1){
		screen->printf(screen, "Invalid args\n");
		return;
	}
	
	char src_name[256];
	strncpy(src_name, args, pos);
	const char * dst_name = &args[pos + 1];
	
	if (!is_file_exists(src_name)){
		screen->printf(screen, "Failed to open source file\n");
		return;
	}
	
	FILE * f_src = fs_open_file(src_name, 'r');
	FILE * f_dst = fs_open_file(dst_name, 'w');
	
	unsigned char buffer[4096];
	int br = 0, bw = 0;
	int fr = 0;
	
	for (;;) {
        fr = fs_read_file(f_src, buffer, sizeof(buffer), &br);
        if (fr || br == 0) 
			break;
			
        fr = fs_write_file(f_dst, buffer);           
        if (fr || bw < br) 
			break; 
    }
	
	fs_close_file(f_src);
	fs_close_file(f_dst);
}

void mkfs(ScreenClass * screen, const char * arg){
	int drive = arg[0] - '0';
	
	mount_drive_and_mkfs_if_needed(screen, drive);
}

void manage_command(ScreenClass * screen, char * current_path, const char * input){
	#define COMMAND(cmd) strcmp(input, cmd) == 0
	#define COMMAND_WITH_ARGS(cmd) strncmp(input, cmd, strlen(cmd)) == 0
	#define CHECK_FILE_EXIST(filename) if (!is_file_exists(filename)){\
			screen->printf(screen, "Failed to open file.\n");\
			return;\
		}\

	fs_getcwd(current_path, 256);

	if (COMMAND("help")){
		screen->printf(screen, "    CROS help:\n"
		" 'ls' - show files and folders in current directory\n"
		" 'mkdir FILE_NAME' - make new directory\n"
		" 'mkfile FILE_NAME' - make new file\n"
		" 'cd FILE_NAME' - change directory\n"
		" 'rm FILE_NAME' - remove file or directory\n"
		" 'cat FILE_NAME' - print file content to console\n"
		" 'edit FILE_NAME' - simple texteditor\n"
		" 'chat' - simple chat application\n"
		" 'chat_server' - simple chat server for 'chat' app\n"
		" 'basic FILE_NAME' - uBASIC interpreter\n"
		" 'paint FILE_NAME' - simple picture editor (WIP)\n"
		" 'ps' - list of processes\n"
		" 'profile PID' - show PID HWI/SWI statistics\n"
		" 'diskinfo' - show info about disk drives & FatFS state\n"
		" 'mkfs DRIVE' - make FatFS at selected disk drive\n"
		" 'disasm 0xADDRESS' - disassemble address\n");
	} else if (COMMAND_WITH_ARGS("edit ")){
		char * args = calloc(256);
		sprintf(args, "%s", &input[strlen("edit ")]);
	
		CHECK_FILE_EXIST(args);
	
		sdk_prc_create_process((unsigned int)app_texteditor, "edit", args, 0);
	} else if (COMMAND_WITH_ARGS("paint ")){
		char * args = calloc(256);
		sprintf(args, "%s", &input[strlen("paint ")]);
	
		CHECK_FILE_EXIST(args);
	
		sdk_prc_create_process((unsigned int)app_paint, "paint", args, 0);
	} else if (COMMAND_WITH_ARGS("basic ")){
		char * args = calloc(256);
		sprintf(args, "%s", &input[strlen("basic ")]);
	
		CHECK_FILE_EXIST(args);
		
		sdk_prc_create_process((unsigned int)app_basic, "basic", args, screen->getCanvas(screen));
	} else if (COMMAND_WITH_ARGS("cp ")){		
		cp(screen, &input[strlen("cp ")]);
	} else if (COMMAND("chat")){
		sdk_prc_create_process((unsigned int)app_chat, "chat", 0, 0);
	} else if (COMMAND("chat_server")){
		sdk_prc_create_process((unsigned int)app_chat_server, "chat_server", 0, 0);
	} else if (COMMAND("memprofile")){
		sdk_prc_create_process((unsigned int)app_memprof, "memprofile", 0, 0);
	} else if (COMMAND("ls")){
		ls(screen, current_path);
	} else if (COMMAND("ps")){
		ps(screen);
	} else if (COMMAND("diskinfo")){
		diskinfo(screen);
	} else if (COMMAND_WITH_ARGS("profile ")){
		profile(screen, &input[strlen("profile ")]);
	} else if (COMMAND_WITH_ARGS("mkfs ")){
		mkfs(screen, &input[strlen("mkfs ")]);
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
		cd(screen, current_path, &input[strlen("cd ")]);
	} else if (COMMAND_WITH_ARGS("rm ")){
		int res = fs_unlink(&input[strlen("rm ")]);
		if (res != FS_OK)
			screen->printf(screen, "Failed to remove file or directory.\n");
	} else if (COMMAND_WITH_ARGS("disasm ")){
		char * args = calloc(256);
		sprintf(args, "%s", &input[strlen("disasm ")]);
		sdk_prc_create_process((unsigned int)app_disasm, "disasm", args, 0);
	} else if (COMMAND_WITH_ARGS("cat ")){
		cat(screen, &input[strlen("cat ")]);
	} else {
		screen->printf(screen, "Unknown command. Type 'help' for commands list.\n");
	}
}