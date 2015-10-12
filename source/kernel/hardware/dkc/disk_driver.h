#ifndef _DISC_DRIVER_H_
#define _DISC_DRIVER_H_

#include <stddef_shared.h>
#include <stdint_shared.h>

typedef enum HW_DKC_FUNC{
	HW_DKC_FUNC_SETIRQMODE = 0,
	HW_DKC_FUNC_QUERY = 1,
	HW_DKC_FUNC_READSECTOR = 2, 
	HW_DKC_FUNC_WRITESECTOR = 3
}HW_DKC_FUNC;

typedef enum HW_DKC_INTS{
	HW_DKC_INTS_FINISHED = 0,
	HW_DKC_INTS_MOUNT = 1,
	HW_DKC_INTS_UNMOUNT = 2
}HW_DKC_INTS;

typedef enum HW_DKC_ERROR{
	HW_DKC_ERROR_NOMEDIA = 1,
	HW_DKC_ERROR_BUSY = 2,
	HW_DKC_ERROR_WRITEPROTECTED = 3,
	HW_DKC_ERROR_INVALIDSECTOR = 4,
	HW_DKC_ERROR_UNKNOWN = 5
} HW_DKC_ERROR;

typedef struct DiskQuery{
	unsigned int errorCode;
	unsigned int status;
	unsigned int numSectors;
	unsigned int sectorSize;
}DiskQuery;

void hw_dkc_setIrqMode(bool mode);
DiskQuery hw_dkc_query(uint32_t diskNum);
void hw_dkc_readSector(uint32_t diskNum, uint32_t sectorNum, char * outData, uint32_t size);
void hw_dkc_writeSector(uint32_t diskNum, uint32_t sectorNum, char * outData, uint32_t size);

void hw_dkc_handleInterrupt(unsigned int reason, uint32_t data0, uint32_t data1);

#endif 