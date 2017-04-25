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

typedef struct FOLDER{
	void * folder;
} FOLDER;

typedef struct FILEINFO{
	unsigned int	fsize;			/* File size */
	int	fdate;						/* Last modified date */
	int	ftime;						/* Last modified time */
	short fattrib;					/* Attribute */
	unsigned char fname[13];		/* Short file name (8.3 format) */
	char ftype;						/* d - directory, f - file */
} FILEINFO;

int fs_mount_drive(int drive_id);
int fs_make_filesystem(int drive_id);
FILE * fs_open_file(const char* path, const char mode);
void fs_close_file(FILE* file);
int fs_write_file(FILE* file, const char * buf);
int fs_read_file(FILE* file, char* buf, unsigned int size, unsigned int* readed_bytes);

int fs_mkdir(const char* path);
int fs_remove(const char* path);

FOLDER * fs_opendir(const char* path);
void fs_closedir(FOLDER* folder);
int fs_readdir(FOLDER* folder, FILEINFO* fileinfo);

int fs_unlink(const char* path);
int fs_chdir(const char* dir);
int fs_chdir(const char* dir);
int fs_getcwd(char * cwd_str, int str_len);
unsigned int fs_getsize(FILE* file);

#endif