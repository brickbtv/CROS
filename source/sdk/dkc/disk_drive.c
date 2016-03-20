#include "disk_drive.h"

#include "sdk/syscall_def.h"

void sdk_dkc_read(u32 diskNum, u32 sectorNum, void * data, int size)
{
	while (!sdk_dkc_isReady(diskNum)){}
	
	app_syscall4(syscall_dkc_read, (u32)diskNum, (u32)sectorNum, (u32)data, (u32)size);
	
	while (!sdk_dkc_isReady(diskNum)){}
}

void sdk_dkc_write(u32 diskNum, u32 sectorNum, const void * data, int size)
{
	while (!sdk_dkc_isReady(diskNum)){}
	
	app_syscall4(syscall_dkc_write, (u32)diskNum, (u32)sectorNum, (u32)data, (u32)size);
	
	while (!sdk_dkc_isReady(diskNum)){}
}

void sdk_dkc_set_flags(u32 diskNum, u32 flags)
{
	//app_syscall2(kSysCall_diskDriveSetFlags, (u32)diskNum, (u32)flags);
}

//int sdk_dkc_get_flags(u32 diskNum)
//{
	//return app_syscall1(kSysCall_diskDriveGetFlags, (u32)diskNum);
//}

int sdk_dkc_get_disk_info(u32 diskNum, DiskQuery * di)
{	
	return app_syscall2(syscall_dkc_getInfo, (u32)diskNum, (u32)di);
}

int sdk_dkc_isReady(u32 diskNum){
	return app_syscall1(syscall_dkc_isReady, (u32)diskNum);
}