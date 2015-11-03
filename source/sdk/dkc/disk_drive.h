#ifndef _DISK_DRIVE_H_
#define _DISK_DRIVE_H_

#include "kernel/hardware/dkc/disk_driver.h"
#include <stddef_shared.h>
#include <stdint_shared.h>

void sdk_dkc_read(u32 diskNum, u32 sectorNum, char * data, int size);
void sdk_dkc_write(u32 diskNum, u32 sectorNum, const char * data, int size);
//void sdk_dkc_set_flags(u32 diskNum, u32 flags);
//int sdk_dkc_get_flags(u32 diskNum);

int sdk_dkc_get_disk_info(u32 diskNum, DiskQuery * di);
int sdk_dkc_isReady(u32 diskNum);

#endif