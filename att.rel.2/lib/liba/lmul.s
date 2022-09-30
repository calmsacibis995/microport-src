	.file	"lmul.s"
/	@(#)	1.2
/ C long multiply

/
/	called for long a,b,c
/		a  = b * c
/
/	b in bx/cx
/	c in ax/dx
/	result returned in ax/dx
/
/	can trash all non-segment registers
/
/	algorithm:
/		If you think of the two numbers being multiplied
/		as unsigned two-digit numbers base 2**16, then
/		(assuming unsigned * is used)
/		a trivial algorithm for the lowest two digits
/		of their product is:
/			a = b(LW) * c(LW)
/			    + 2**16 * b(LW) * c(HW)
/			    + 2**16 * b(HW) * c(LW)
/

#define	RHS_low		-2(%bp)

	.text
	.globl	_lmul

_lmul:
	enter	$2,$0

	mov	%ax,%si		/ save b(LW) temporarily in %si
	mov	%dx,%di		/ save b(HW) in %di
	mul	%bx		/ b(LW) * c(LW)
	mov	%ax,RHS_low	/ finished with a(LW), so squirrel it away
	mov	%si,%ax		/ last use of b(LW), so %si now avail.
	mov	%dx,%si		/ %si now accumulating a(HW)
	mul	%cx		/ b(LW) * c(HW)
	add	%ax,%si		/ ignore any overflow in %dx
	mov	%di,%ax		/ b(HW)
	mul	%bx		/ b(HW) * c(LW)
	add	%ax,%si		/ a(HW) now finished
	mov	RHS_low,%ax	/ return a(LW)
	mov	%si,%dx		/ return a(HW)

	leave
#if LARGE_M | HUGE_M | COMPACT_M
	lret
#else
	ret
#endif
