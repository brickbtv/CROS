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
	syscall_prc_create_process,
	syscall_prc_sleep,
	syscall_prc_sleep_until_new_messages,
	syscall_prc_getCurrentProcessScreenInfo,
	syscall_prc_getCurrentProcessHeapPointer,
	syscall_prc_haveNewMessages,
	syscall_prc_getNextMessage,
	syscall_prc_lock,
	syscall_prc_unlock,
	syscall_prc_is_focused,
	syscall_prc_die,
	syscall_get_scheduler_list,
	syscall_get_total_memory,
	// clock
	syscall_clk_readTimeSinceBoot,
	syscall_clk_readCountdownTimer,
	syscall_clk_setCountdownTimer,
	// network
	syscall_nic_address,
	syscall_nic_debug,
	syscall_nic_send,
	syscall_nic_recv,
	// disk drive
	syscall_dkc_read,
	syscall_dkc_write,
	syscall_dkc_getInfo,
	syscall_dkc_isReady
} SYSCALL_CBACKS;

static char * SYSCALL_NAMES_DUMMY[23] = {
		"syscall_prc_create_process",
		"syscall_prc_sleep",
		"syscall_prc_sleep_until_new_messages",
		"syscall_prc_getCurrentProcessScreenInfo",
		"syscall_prc_getCurrentProcessHeapPointer",
		"syscall_prc_haveNewMessages",
		"syscall_prc_getNextMessage",
		"syscall_prc_lock",
		"syscall_prc_unlock",
		"syscall_prc_is_focused",
		"syscall_prc_die",
		"syscall_get_scheduler_list",
		"syscall_get_total_memory",
		// clock
		"syscall_clk_readTimeSinceBoot",
		"syscall_clk_readCountdownTimer",
		"syscall_clk_setCountdownTimer",
		// network
		"syscall_nic_debug",
		"syscall_nic_send",
		"syscall_nic_recv",
		// disk drive
		"syscall_dkc_read",
		"syscall_dkc_write",
		"syscall_dkc_getInfo",
		"syscall_dkc_isReady"
	};
#endif