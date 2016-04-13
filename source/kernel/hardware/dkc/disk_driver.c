#include "disk_driver.h"

#include "hwi/hwi.h"
#include "kernel/kernel.h"
#include "kernel/hardware/cpu/cpu_driver.h"

void hw_dkc_init(void){
	hw_dkc_setIrqMode(true);
}

static int hw_dkc_busy = 0;

void hw_dkc_setIrqMode(bool mode){
	hw_HwiData data;
	data.regs[0] = (uint32_t)mode;
	
	hwi_call(HW_BUS_DKC, HW_DKC_FUNC_SETIRQMODE, &data);
}

DiskQuery hw_dkc_query(uint32_t diskNum){
	hw_HwiData data;
	data.regs[0] = diskNum;
	
	HWERROR err = hwi_call(HW_BUS_DKC, HW_DKC_FUNC_QUERY, &data);
	
	DiskQuery query;
	query.status = data.regs[0];
	query.numSectors = data.regs[1];
	query.sectorSize = data.regs[2];
	
	query.errorCode = err;
	
	return query;
}

int hw_dkc_is_ready(uint32_t diskNum){
	return (hw_dkc_busy == 0);
}

void hw_dkc_read_sync(uint32_t diskNum, uint32_t sectorNum, void * outData, uint32_t size){
	//while (hw_dkc_busy){hw_cpu_halt();}
	hw_dkc_readSector(diskNum, sectorNum, outData, size);
}

void hw_dkc_write_sync(uint32_t diskNum, uint32_t sectorNum, void * outData, uint32_t size){
	//while (hw_dkc_busy){hw_cpu_halt();}
	hw_dkc_writeSector(diskNum, sectorNum, outData, size);
}

void hw_dkc_readSector(uint32_t diskNum, uint32_t sectorNum, void * outData, uint32_t size){
	hw_HwiData data;
	data.regs[0] = (sectorNum << 8) | diskNum;
	data.regs[1] = (uint32_t) outData;
	data.regs[2] = size;
	
	hwi_call(HW_BUS_DKC, HW_DKC_FUNC_READSECTOR, &data);
	hw_dkc_busy = 1;
}

void hw_dkc_writeSector(uint32_t diskNum, uint32_t sectorNum, void * outData, uint32_t size){
	
	//krn_debugLogf("There: %d", size);
	
	hw_HwiData data;
	data.regs[0] = (sectorNum << 8) | diskNum;
	data.regs[1] = (uint32_t) outData;
	data.regs[2] = size;
	
	hwi_call(HW_BUS_DKC, HW_DKC_FUNC_WRITESECTOR, &data);
	hw_dkc_busy = 1;
}

void hw_dkc_handleInterrupt(unsigned int reason, uint32_t data0, uint32_t data1){
	switch(reason){
		case HW_DKC_INTS_FINISHED:
			hw_dkc_busy = 0;
			break;
			
		case HW_DKC_INTS_MOUNT:
			krn_debugLogf("DKC0: mounted %d", data1);
			break;
		
		case HW_DKC_INTS_UNMOUNT:
			krn_debugLogf("DKC0: unmounted %d", data1);
			break;
			
		default:
			krn_debugBSODf("DKC0 interruption", "Unknown interruption - %d", reason);
	}
}