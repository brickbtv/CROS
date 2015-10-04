
extern _krn_init
extern _krn_start
extern _krn_handleInterrupt

; interruptions
; Address: 0x0	Size: 8 bytes
.text
.word _reset			; 0x0	start
.word _interrupts_all	; 0x4	interruptions handler

; reserve cpu registers context
; Address: 0x8	Size: 196 bytes
.zero 196		; default registers set

; init function
public _reset
_reset:

	bl _krn_init
	ctxswitch [r0]
	
	
_interrupts_all:
	bl _krn_handleInterrupt
	ctxswitch [r0]
