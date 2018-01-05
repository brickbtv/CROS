#ifndef _CTX_H_
#define _CTX_H_

#define CPU_REG_DS 11
#define CPU_REG_SP 13
#define CPU_REG_PC 15

typedef enum Usermode{
	USERMODE_SUPERVISOR = (1<<26),
	USERMODE_USER = 0
}Usermode;

/*!
*	wrapper for registers set.
*/
typedef struct Ctx{
	int gregs[16];
	int rim0, rim1;
	int flags;
	double fregs[16];
} Ctx;

#endif