#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <stddef_shared.h>

int app_syscall0(
	__reg("r10") int)
INLINEASM("\t\
swi");

int app_syscall1(
	__reg("r10") int,
	__reg("r0") int)
INLINEASM("\t\
swi");

int app_syscall2(
	__reg("r10") int,
	__reg("r0") int,
	__reg("r1") int)
INLINEASM("\t\
swi");

int app_syscall3(
	__reg("r10") int,
	__reg("r0") int,
	__reg("r1") int,
	__reg("r2") int)
INLINEASM("\t\
swi");

int app_syscall4(
	__reg("r10") int,
	__reg("r0") int,
	__reg("r1") int,
	__reg("r2") int,
	__reg("r3") int)
INLINEASM("\t\
swi");

void syscall_clk_readTimeSinceBoot(void);

#endif