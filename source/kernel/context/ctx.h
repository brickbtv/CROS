#ifndef _CTX_H_
#define _CTX_H_

#define CPU_REG_SP 13
#define CPU_REG_PC 15
#define CPU_FLAGSREG_SUPERVISOR (1<<26)

/*!
*	wrapper for registers set.
*/
typedef struct Ctx{
	int gregs[16];
	int flags;
	double fregs[16];
} Ctx;

#endif