#ifndef _HWI_H_
#define _HWI_H_

#include <stdint_shared.h>
#include <stddef_shared.h>
#include "hardware/hardware.h"

typedef int32_t HWERROR;
typedef uint8_t hw_BusId;

/*!
*	wrapper for "hwi" instruction input and output data.
*/
typedef struct {
	uint32_t regs[4];
} hw_HwiData;

/*!
*	Simple wrapper to "hwi" assembly instrucion.
*/
HWERROR hwi_call(hw_BusId bus, uint32_t func, hw_HwiData* data);

#endif