/*
 * Fixed devices bus numbers
 */

#ifndef _HWI_H_
#define _HWI_H_

#include <stdint_shared.h>
#include <stddef_shared.h>
#include "hardware/hardware.h"

typedef int32_t HWERROR;
typedef uint8_t hw_BusId;

typedef struct {
	uint32_t regs[4];
} hw_HwiData;

HWERROR hwi_call(hw_BusId bus, uint32_t func, hw_HwiData* data);

#endif