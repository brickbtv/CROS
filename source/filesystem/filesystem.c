#include "filesystem.h"
#include "filesystem/fatfs/src/ff.h"

#include <details/memdetails.h>
#include <string_shared.h>
#include "sdk/os/debug.h"

int fs_mount_drive(int drive_id){
	FATFS * fs = malloc(sizeof(FATFS));
	FRESULT res = f_mount(fs, "", 1);
	if (res == FR_NO_FILESYSTEM){
		return FS_NO_FILESYSTEM;
	} 
	
	if (res != FR_OK){
		return FS_UNKNOWN_ERROR;
	}
	
	return FS_OK;
}

int fs_make_filesystem(){
	FRESULT res = f_mkfs("", 0, 0);
	if (res != FR_OK){
		return FS_UNKNOWN_ERROR;
	}
	
	return FS_OK;
}

FILE * fs_open_file(const char* path, const char mode){
	FILE * file = malloc(sizeof(FILE));
	FIL * fil = malloc(sizeof(FIL));
	file->fil = (void*)fil;
	
	int open_mode = 0;
	if (mode == 'w'){
		open_mode = FA_CREATE_ALWAYS | FA_WRITE;
	} else if (mode == 'r'){
		open_mode = FA_READ;
	} else {
		return 0;
	}
	
	FRESULT res = f_open((FIL*)file->fil, path, open_mode);
	if (res != FR_OK){
		return 0;
	}
	
	return file;
}

void fs_close_file(FILE* file){
	f_close((FIL*)file->fil);
	
	if (file != 0 && file->fil != 0){
		free(file->fil);
		free(file);
		
		file->fil = 0;
		file = 0;
	}
}

int fs_write_file(FILE* file, const char * buf){
	UINT bw; 
	f_write((FIL*)file->fil, buf, strlen(buf), &bw);
	
	if (bw != strlen(buf)){
		return FS_UNKNOWN_ERROR;
	}
	
	return FS_OK;
}

int fs_read_file(FILE* file, char* buf, unsigned int size, unsigned int* readed_bytes){
	char b[1024];	
	if (size > 1024){
		return FS_INVALID_PARAM;
	}
	
	UINT br; 
	FRESULT res = f_read((FIL*)file->fil, b, size, &br);
	*readed_bytes = br;
	strcpy(buf, b);
	
	if (res != FR_OK){
		return FS_UNKNOWN_ERROR;
	}
	
	return FS_OK;
}

int fs_mkdir(const char* path){
	FRESULT res = f_mkdir(path);
	if (res == FR_EXIST){
		return FS_EXIST;
	}
	if (res != FR_OK){
		return FS_UNKNOWN_ERROR;
	}
	
	return FS_OK;
}

int fs_remove(const char* path){
	FRESULT res = f_unlink(path);
	if (res != FR_OK){
		return FS_UNKNOWN_ERROR;
	}
	return FS_OK;
}

FOLDER * fs_opendir(const char* path){
	FOLDER* folder = malloc(sizeof(FOLDER));
	DIR* dir = malloc(sizeof(DIR));
	FRESULT res = f_opendir(dir, path);
	if (res != FR_OK){
		return 0;
	}
	
	folder->folder = dir;
	
	return folder;
}

void fs_closedir(FOLDER* folder){
	if (folder && folder->folder){
		free(folder->folder);
		folder->folder = NULL;
		free(folder);
		folder = NULL;
	}
}

int fs_readdir(FOLDER* folder, FILEINFO* fileinfo){
	FILINFO inf;
	memset(&fileinfo->fname[0], 0, sizeof(fileinfo->fname));
	FRESULT res = f_readdir(folder->folder, &inf);
	if (res != FS_OK){
		return FS_UNKNOWN_ERROR;
	}
	
	fileinfo->fattrib = inf.fattrib;
	fileinfo->fdate = inf.fdate;
	strcpy(fileinfo->fname, inf.fname);
	fileinfo->fsize = inf.fsize;
	fileinfo->ftime = inf.ftime;
	
	return FS_OK;	
}

int fs_unlink(const char* path){
	FRESULT res = f_unlink(path);
	if (res != FR_OK){
		return FS_UNKNOWN_ERROR;
	}
	
	return FS_OK;
}