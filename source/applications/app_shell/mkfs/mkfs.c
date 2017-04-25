#include "mkfs.h"

#include <utils/filesystem/filesystem.h>

#define APP_SHELL_NO_FS "No file system found.\n" \
						"Marking drive 0. It takes 1-2 minutes.\n"
#define APP_SHELL_FAILED_FS "Failed to make filesytem. Abort.\n"
#define APP_SHELL_DONE "Done.\n"
#define APP_SHELL_FAILED_MOUNT "Failed to mount drive. It's mounted? Abort.\n"

int mount_drive_and_mkfs_if_needed(ScreenClass * screen){
	int res = fs_mount_drive(0);
	if (res != FS_OK){
		if (res == FS_NO_FILESYSTEM){
			screen->printf(screen, APP_SHELL_NO_FS);
			int res_mkfs = fs_make_filesystem(0);
			if (res_mkfs != FS_OK){
				screen->printf(screen, APP_SHELL_FAILED_FS);
				return 1;
			} else {
				screen->printf(screen, APP_SHELL_DONE);
			}
		} else {
			screen->printf(screen, APP_SHELL_FAILED_MOUNT);
			return 2;
		}
	} 
	
	return 0;
}