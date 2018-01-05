.text

;
; Full hwi call
public _asm_test
_asm_test:
	nop
	nop
	nop
_test_l:	
	nop
	bl _test_l
	nop
	hwf
	swi
	nop
	mrs r2
	msr r3
	nop
	push {lr}
	nop
	not r12, r14
	nop
	smul r0:r1, r2 ,r3
	smul r0, r2, 65525
	nop
	add r0, r1, r2
	sub r0, r1, 123
	sra r0, r1, 65530
	nop
	
	mov r0, 19
	mov r1, r2
	mov r2, 120
	mov r2, 65530
	mov r2, 10000003