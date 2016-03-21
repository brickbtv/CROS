#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#define FS_OK 				0 
#define FS_UNKNOWN_ERROR	1
#define FS_WRONG_DRIVE		2
#define FS_NO_FILESYSTEM	3
#define FS_INVALID_PARAM 	4
#define FS_EXIST			5

typedef struct FILE{
	void * fil;
} FILE;

int fs_mount_drive(int drive_id);
int fs_make_filesystem();
FILE * fs_open_file(const char* path, const char mode);
void fs_close_file(FILE* file);
int fs_write_file(FILE* file, const char * buf);
int fs_read_file(FILE* file, char* buf, unsigned int size, unsigned int* readed_bytes);

int fs_mkdir(const char* path);
int fs_remove(const char* path);

#endif