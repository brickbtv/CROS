#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include "stdint_shared.h"

#define HWBUS_DEFAULTDEVICES_MAX 6

typedef enum HW_BUS{
	HW_BUS_CPU = 0,
	HW_BUS_CLK = 1,
	HW_BUS_SCR = 2,
	HW_BUS_KYB = 3, 
	HW_BUS_NIC = 4,
	HW_BUS_DKC = 5
}HW_BUS;

#define HW_SCR_FUNC_SCREENINFO 0
#define HW_SCR_FUNC_MAPSCREENBUFFER 1
 

#define HW_NIC_FUNC_SEND 0 // TODO: it changes to 1, after next release of DevKit

#define HWERR_SUCCESS 0x0
#define HWERR_DEFAULT_DEVICENOTFOUND 0x80000001
#define HWERR_DEFAULT_INVALIDFUNCTION 0x80000002
#define HWERR_DEFAULT_INVALIDMEMORYADDRESS 0x80000003

#define HWDEFAULT_FUNC_QUERYINFO 0x80000000
#define HWDEFAULT_FUNC_QUERYDESC 0x80000001

void hw_initAll(void);

void hw_handleInterrupt(int bus_and_reason, u32 data0, u32 data1, u32 data2, u32 data3);

#endif