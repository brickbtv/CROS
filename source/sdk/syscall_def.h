#ifndef _SYSCALL_DEF_H_
#define _SYSCALL_DEF_H_

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

typedef enum SYSCALL_CBACKS{
	// processes
	syscall_prc_sleep,
	syscall_prc_getCurrentProcessScreenInfo,
	// clock
	syscall_clk_readTimeSinceBoot,
	syscall_clk_readCountdownTimer,
	syscall_clk_setCountdownTimer,
	// network
	syscall_nic_debug
} SYSCALL_CBACKS;

#endif