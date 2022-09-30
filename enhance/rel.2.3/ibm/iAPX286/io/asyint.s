	.file	"asyint.s"
/ Copyright 1985 by Microport. All Rights Reserved.
/ *uportid = "@(#)asyint.s	Microport Rev Id  2.3.2 4/14/87";
/
/ @(#)asyint.s	1.00
/
/ Special assembler handler for asy driver, for speed
/ Channels 0 and 1
/ M000 - uport!mark add interrupt channels 2 & 3  (INT5 & INT7)

#include	"sys/param.h"
#include	"sys/mmu.h"
#include	"sys/trap.h"
#include	"sys/8259.h"

	.globl	sd_array

	.text

/
/ 	SIO Channel 0 interrupts enter here
/		uses INT 4 line and chain
	.globl	_asyintr0
_asyintr0:
	cli
	pusha				/ save all the regular registers, we use them all
/ don't need to save flags, dev trap already did that
	push	%ds			/ save the segment registers
	push	%es
si0loop:
	mov	$<s>si_chain,%ax
	mov	%ax,%ds
	mov	$si_chain+16,%si 	/ ds:si = &si_chain[ 4 ]
	test	%ds:2(%si)		/ test seg of chain head
	jz	si0_ret			/ nothing in chain, shouldn't happen
	lds	%ds:(%si),%si		/ ds:si = first sd in chain
	les	%ds:18(%si),%di		/ es:di = common data for int chain
	mov	$0,%es:(%di)		/ clear activity flag
	cld				/ make sure forward
	lcall	asyfast	/ call the handler
	les	%ds:18(%si),%di		/ es:di = common data for int chain
	test	%es:(%di)
	jnz	si0loop			/ if was activity, poll again
si0_ret:
	jmp	si_ret		/ jump to common asy return code


/
/ 	SIO Channel 2 interrupts enter here
/		uses INT 5 line and chain
	.globl	_asyintr2
_asyintr2:
	cli
	pusha				/ save all the regular registers, we use them all
/ don't need to save flags, dev trap already did that
	push	%ds			/ save the segment registers
	push	%es
si2loop:
	mov	$<s>si_chain,%ax
	mov	%ax,%ds
	mov	$si_chain+20,%si 	/ ds:si = &si_chain[ 5 ]
	test	%ds:2(%si)		/ test seg of chain head
	jz	si2_ret			/ nothing in chain, shouldn't happen
	lds	%ds:(%si),%si		/ ds:si = first sd in chain
	les	%ds:18(%si),%di		/ es:di = common data for int chain
	mov	$0,%es:(%di)		/ clear activity flag
	cld				/ make sure forward
	lcall	asyfast	/ call the handler
	les	%ds:18(%si),%di		/ es:di = common data for int chain
	test	%es:(%di)
	jnz	si2loop			/ if was activity, poll again
si2_ret:
	jmp	si_ret		/ jump to common asy return code


/
/ 	SIO Channel 3 interrupts enter here
/		uses INT 7 line and chain
	.globl	_asyintr3
_asyintr3:
	cli
	pusha				/ save all the regular registers, we use them all
/ don't need to save flags, dev trap already did that
	push	%ds			/ save the segment registers
	push	%es
si3loop:
	mov	$<s>si_chain,%ax
	mov	%ax,%ds
	mov	$si_chain+28,%si 	/ ds:si = &si_chain[ 7 ]
	test	%ds:2(%si)		/ test seg of chain head
	jz	si3_ret			/ nothing in chain, shouldn't happen
	lds	%ds:(%si),%si		/ ds:si = first sd in chain
	les	%ds:18(%si),%di		/ es:di = common data for int chain
	mov	$0,%es:(%di)		/ clear activity flag
	cld				/ make sure forward
	lcall	asyfast	/ call the handler
	les	%ds:18(%si),%di		/ es:di = common data for int chain
	test	%es:(%di)
	jnz	si3loop			/ if was activity, poll again
si3_ret:
	jmp	si_ret		/ jump to common asy return code

/
/ 	SIO Channel 1 interrupts enter here
/		uses INT 3 line and chain
	.globl	_asyintr1
_asyintr1:
	cli
	pusha				/ save all the regular registers
	push	%ds			/ save the segment registers
	push	%es
si1loop:
	mov	$<s>si_chain,%ax
	mov	%ax,%ds
	mov	$si_chain+12,%si 	/ ds:si = &si_chain[ 3 ]
	test	%ds:2(%si)		/ test seg of chain head
	jz	si1_ret			/ nothing in chain, shouldn't happen
	lds	%ds:(%si),%si		/ ds:si = first sd in chain
	les	%ds:18(%si),%di		/ es:di = common data for int chain
	mov	$0,%es:(%di)		/ clear activity flag
	cld				/ make sure forward
	lcall	asyfast		/ call the fast (asm) asy handler
	les	%ds:18(%si),%di		/ es:di = common data for int chain
	test	%es:(%di)
	jnz	si1loop			/ if was activity, poll again
si1_ret:

/ Common SIO return code
si_ret:		
	pop	%es			/ pop off (possibly modified) seg regs
	pop	%ds
	popa				/ restore general regs
	lret				/ get out of here



