#include "commands.h"

#include "filesystem/filesystem.h"

#include <string_shared.h>

void manage_command(Canvas * canvas, char * current_path, const char * input){
	fs_getcwd(current_path, 256);

	if (strcmp(input, "help") == 0){
		sdk_scr_printf(canvas, "wow! such help! much text!\n"
		" 'ls' - show files and folders in current directory\n"
		" 'mkdir NAME' - make new directory\n"
		" 'mkfile NAME' - make new file\n"
		" 'cd NAME' - change directory\n"
		" 'rm NAME' - remove file or directory\n");
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
		fs_mkdir(&input[strlen("mkdir ")]);
	} else if (strncmp(input, "mkfile ", strlen("mkfile ")) == 0){
		FILE * file = fs_open_file(&input[strlen("mkfile ")], 'w');
		if (file){
			fs_close_file(file);
		} else {
			sdk_scr_printf(canvas, "Failed to create file.\n");
		}
	} else if (strncmp(input, "cd ", strlen("cd ")) == 0){
		fs_chdir(&input[strlen("cd ")]);
		fs_getcwd(current_path, 256);
	} else if (strncmp(input, "rm ", strlen("rm ")) == 0){
		fs_unlink(&input[strlen("rm ")]);
	}
}