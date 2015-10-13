
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
	
	ctxswitch [r0]
	; unexpected context switch. fatal.
	bl _krn_unexpectedContextSwitch

; handle all interruption here:		
_interrupts_all:
	; save bus and reason before call handler
	str [_krn_currIntrBusAndReason], ip
	bl _krn_handleInterrupt
	
	ctxswitch [r0]
	
	; unexpected context switch. fatal.
	bl _krn_unexpectedContextSwitch


; variable, which used to save interruption bus and reson before call handler
public _krn_currIntrBusAndReason
_krn_currIntrBusAndReason:
.word 15