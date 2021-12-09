.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm

.global	delay
.global enable_interrupt

delay: 
	li 	$t0,0
	li 	$t1,3846153#4711
while:
	ble 	$a0,$zero,ret 
	nop
	addi 	$a0,$a0,-1	 
loop:
	bge 	$t0,$t1,while
	nop
	addi 	$t0,$t0,1
	j 	loop
	nop
ret:
	jr 	$ra
	nop

enable_interrupt:
	ei
	jr $ra 
	nop