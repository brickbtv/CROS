#include "process.h"

/*!
*	Draft of multiprocessing stuff
*/
Process prc_create(const char * name, uint32_t * stackTop,
						uint32_t * entryPoint, Usermode mode){
	Process prc;
	memset(&prc, 0, sizeof(Process));
	strcpy(prc.name, name);
	
	prc.context.gregs[CPU_REG_SP] = (uint32_t)stackTop;
	prc.context.gregs[CPU_REG_PC] = (uint32_t)entryPoint;
	prc.context.flags = mode;
	
	return prc;
}