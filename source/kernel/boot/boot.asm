
extern _krn_init
extern _krn_handleInterrupt
extern _krn_unexpectedContextSwitch

; interruptions
; Address: 0x0	Size: 8 bytes
.text
.word _reset			; 0x0	start
.word _interrupts_all	; 0x4	interruptions handler

; reserve cpu registers context
; Address: 0x8	Size: 196 bytes
.zero 196		; default registers set

; init function
_reset:
	bl _krn_init
	
	; set up double fault catcher
	str [_krn_currIntrBusAndReason], -1
	
	ctxswitch [r0]
	; unexpected context switch. fatal.
	bl _krn_unexpectedContextSwitch

; handle all interruption here:		
_interrupts_all:	
	; Set the previous bus and reason variable
	; This allows us to detect kernel double faults
	ldr r4, [_krn_currIntrBusAndReason]
	str [_krn_prevIntrBusAndReason], r4
	str [_krn_currIntrBusAndReason], ip
	
	bl _krn_handleInterrupt
	
	; We are done with the interrupt servicing, so martk it as so.
	str [_krn_currIntrBusAndReason], -1
	
	ctxswitch [r0]
	
	; unexpected context switch. fatal.
	bl _krn_unexpectedContextSwitch

public _hw_cpu_enableIRQ
_hw_cpu_enableIRQ:
	mrs r0 ; load flags register
	and r0, r0, ~(1<<27) ; clear bit 27
	msr r0 ; and set the flags register to the new value
	mov pc, lr

public _hw_cpu_disableIRQ
_hw_cpu_disableIRQ:
	mrs r0 ; load flags register
	or r0, r0, 1<<27 ; Set bit 27
	msr r0 ; and set the flags register to the new value
	mov pc, lr


; variable, which used to save interruption bus and reson before call handler
public _krn_currIntrBusAndReason
_krn_currIntrBusAndReason:
.word 15


; This variable allows detection of double faults.
; When we are serving an interrupt, if another one happens, it's a double fault.
public _krn_prevIntrBusAndReason
_krn_prevIntrBusAndReason:
.word 15