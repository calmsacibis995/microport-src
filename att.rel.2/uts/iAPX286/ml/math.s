	.file	"math.s"
/
/ @(#)math.s	1.9
/

	.text
#include	"../sys/psl.h"

/
/ min
/	return the minimum of the two arguments
/
/	min( arg0, arg1 )
/	unsigned arg0, arg1;
/
	.globl	min
min:
	push	%bp			/ establish ...
	mov	%sp,%bp			/ ... stack frame
	mov	6(%bp),%ax		/ get arg0
	cmp	8(%bp),%ax		/ arg0 >= arg1 ?
	ja	min1			/ yes, return arg1
	jmp	min2			/ no. arg0 is already in ax, so return
min1:
	mov	8(%bp),%ax		/ return arg1
min2:
	pop	%bp
	lret

/
/ max
/	return the maximum of the two arguments
/
/	max( arg0, arg1 )
/	unsigned arg0, arg1;
/
	.globl	max
max:
	push	%bp			/ establish ...
	mov	%sp,%bp			/ ... stack frame
	mov	6(%bp),%ax		/ get arg0
	cmp	8(%bp),%ax		/ arg0 < arg1 ?
	jb	max1			/ yes, return arg1
	jmp	max2			/ no. arg0 is already in ax, so return
max1:
	mov	8(%bp),%ax		/ return arg1
max2:
	pop	%bp
	lret


/
/ savefp
/	save the floating point registers into the address
/	specified
/
	.globl	savefp
savefp:
	push	%bp			/ establish ...
	mov	%sp,%bp			/ ... stack frame
	les	6(%bp),%di		/ load save address
	fnsave	%es:0(%di)		/ save all the registers
	pop	%bp
	lret

/
/ restorefp
/	restore the floating point registers from the address
/	specified
/
	.globl	restorefp
restorefp:
	push	%bp			/ establish ...
	mov	%sp,%bp			/ ... stack frame
	les	6(%bp),%di		/ load restore address
	frstor	%es:0(%di)		/ restore all the registers
	pop	%bp
	lret

/
/ clts
/	return the value of the task switched flag in the MSW,
/	then reset the task switched bit
/
	.globl	clts
clts:
	smsw	%ax			/ get the MSW
	and	$MS_TS,%ax		/ mask to just the ts flag
	clts				/ make sure it is cleared
	lret
