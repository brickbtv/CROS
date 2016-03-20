/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "kernel/hardware/dkc/disk_driver.h"
#include "sdk/dkc/disk_drive.h"
#include "sdk/os/debug.h"

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

#define HW_DKC_MAXDISKS 4

static int get_status(int diskNum)
{
	return 0;
}

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return get_status(pdrv);
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DiskQuery di;
	sdk_dkc_get_disk_info(pdrv, &di);
	
	sdk_dkc_read(pdrv, sector, buff, count * di.sectorSize);
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DiskQuery di;
	sdk_dkc_get_disk_info(pdrv, &di);
	
	
	//sdk_debug_logf("%d, %d, %d, secs: %d stat: %d", pdrv, sector, count, di.sectorSize, di.status);
	sdk_dkc_write(pdrv, sector, buff, count*di.sectorSize);
	return 0;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DiskQuery di;
	sdk_dkc_get_disk_info(pdrv, &di);

	switch (cmd){
		case GET_SECTOR_SIZE:
			*((int*)buff)=di.sectorSize;
			return RES_OK;
		case GET_SECTOR_COUNT:
			*((int*)buff)=di.numSectors;
			return RES_OK;
		case GET_BLOCK_SIZE:
			*((int*)buff)=di.sectorSize;
			return RES_OK;
		case CTRL_SYNC:
			return RES_OK;
		//case CTRL_ERASE_SECTOR:
			/*
				TODO: Find information about CTRL_ERASE_SECTOR.
					  There's nothing here: http://elm-chan.org/fsw/ff/en/dioctl.html
			*/
		//	LOG("FatFS CTRL_ERASE_SECTOR not supported yet");
		//	return RES_PARERR;
		default:
			return RES_PARERR;
	}

	return RES_PARERR;
}

DWORD get_fattime(void)
{
	/*
		TODO: Returns the current date/time
	*/
	return 0;//
}

#endif
