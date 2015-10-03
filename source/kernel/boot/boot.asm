; interruptions
; Address: 0x0	Size: 8 bytes
.text
.word _reset	; 0x0	Reset interrupt handler address
.word 0			; 0x4	TODO: impl interruptions handler

; reserve cpu registers context
; Address: 0x8	Size: 196 bytes
.zero 196		; default registers set

; init function
public _reset
_reset:
	
	