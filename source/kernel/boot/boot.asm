
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

; variable, which used to save interruption bus and reson before call handler
public _krn_currIntrBusAndReason
_krn_currIntrBusAndReason:
.word 15


; This variable allows detection of double faults.
; When we are serving an interrupt, if another one happens, it's a double fault.
public _krn_prevIntrBusAndReason
_krn_prevIntrBusAndReason:
.word 15

;
; Read only data (after we set MMU)
;
.rodata

;
; Process runtime information
; This is patched by the linker when building a ROM file, 
; and information about the size of the program:
; Contents are:
; 	4 bytes - readOnlyAddress (where code and .rodata starts)
;	4 bytes - readOnlySize (size of the read only portion)
;	4 bytes - readWriteAddress ( where read/write data starts)
;	4 bytes - readWriteSize (size of the read/write data)
;	4 bytes - sharedReadWriteAddress ( where shared read/write data starts)
;	4 bytes - sharedReadWriteSize (size of the shared read/write data)
public _processInfo
_processInfo:
.zero 24

.section ".apcpudebug"
