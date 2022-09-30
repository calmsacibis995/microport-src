	.file	"ldivmod.s"
/	@(#)	1.2

/	C long div and mod (/ and %)
/
/	lmod called for long a,b,c
/		a = b % c
/	b in ax/dx
/	c in bx/cx
/	return result in ax/dx
/

#define	SAVEBHW		-2(%bp)
#define	SAVEBLW		-4(%bp)
#define	SAVECHW		-6(%bp)
#define	SAVECLW		-8(%bp)

#define LREMFLAG	$4
#define	DVNDFLAG	$2
#define	DVSRFLAG	$1
#define	BOTHFLAG	$3

	.globl	_lmod
_lmod:
	enter	$8,$0
	mov	LREMFLAG,%di		/ set lrem flag, clear sign flag
	jmp	chkdvnd

	.globl	_ulmod
_ulmod:
	enter	$8,$0
	mov	LREMFLAG,%di		/ set lrem flag, clear sign flag
	and	%cx,%cx			/ set condition flags for main code
	jmp	dvsrok

/
/	uldiv called for unsigned long a,b,c
/	(NOTE: only one of b or c being unsigned
/		causes the expression to be calculated as unsigned
/		as per standard C semantics)
/	ldiv called for long a,b,c
/		a = b / c
/	b in ax/dx
/	c in bx/cx
/	return result in ax/dx
/
	.globl	_uldiv
_uldiv:
	enter	$8,$0
	xor	%di,%di		/ merely clear the sign flags
	and	%cx,%cx		/ set condition flags for main code
	jmp	dvsrok		/ since the main part of ldiv
				/ is an unsigned algorithm

	.globl	_ldiv
_ldiv:
	enter	$8,$0

	xor	%di,%di		/clear sign flags
chkdvnd:
	and	%dx,%dx		/ is dividend neg?
	jns	dvndok
	xor	DVNDFLAG,%di	/ set neg dividend flag
	neg	%dx		/ and change sign
	neg	%ax		/ of dividend
	sbb	$0,%dx

dvndok:
/ dividend now unsigned
/	NOTE:	if dividend was 0x80000000, it still is,
/		but that is the correct unsigned value.
	and	%cx,%cx		/ is divisor neg?
	jns	dvsrok
	xor	DVSRFLAG,%di	/ set neg divisor flag
	neg	%cx		/ and change sign
	neg	%bx		/ of divisor
	sbb	$0,%cx

dvsrok:
/ divisor now unsigned (see NOTE for dividend)
/
/ Now determine how hard it is to divide.
/ Ultimately, the divisor must be reduced to one word in size,
/ so a divisor whose high word is already zero is much easier
/ to handle.
/
/ Condition codes for divisor high word are already set,
/ either by the 'and' for checking for negativity
/ or by the 'sbb' that fixes up the high word when needed.
/ (the jumps don't change the flags)
/

	jnz	fulldiv		/ jump if c[HW] != 0

/
/ c[HW] is zero.
/
/ This piece of code assumes that the case where
/ c[LW] > b[HW] is a high use path.
/ If it is determined that it is not a high use path
/ or if we get super stingy on code space, then the code
/ for this special case can be eliminated, since the
/ code for c[LW] <= b[HW] would get the right answer.
/
	cmp	%bx,%dx		/ this is %dx - %bx
	jnb	qtwodiv		/ use two word quotient code
				/ if c[LW] <= b[HW]
	div	%bx
	test	LREMFLAG,%di		/ is remainder desired?
	jnz	fixsrem			/ then go return it instead of quotient
	xor	%dx,%dx		/ one word quotient, so zero a[HW]
	jmp	fixsign

qtwodiv:
/ c[HW] == 0 && c[LW] <= b[HW]
/ This is the only case where the quotient has two significant words.
/
/ The division takes places as if we were dealing with numbers
/ base 256, where the dividend is a two-digit number and
/ the divisor is a one-digit number.
/
	mov	%ax,%si		/ squirrel away b[LW]
	mov	%dx,%ax		/ set up first divide
	xor	%dx,%dx
	div	%bx		/ producing a[HW] in %ax
				/ and partial remainder for
				/ second divide in %dx (the right place)
	xchg	%ax,%si		/ save a[HW] and set up second divide
	div	%bx		/ producing a[LW] in %ax
	test	LREMFLAG,%di		/ is remainder desired?
	jnz	fixsrem			/ then go return it instead of quotient
	mov	%si,%dx		/ a now in ax/dx
	jmp	fixsign

fixsrem:
	mov	%dx,%ax
	xor	%dx,%dx
	and	DVNDFLAG,%di
	jmp	fixsign

fulldiv:
/
/ C[HW] != 0
/ so things get a little trickier to think about.
/ Now we think of the numbers base 256 (2**8).
/ So, in that form, the divisor is either a three or four digit number.
/ If the divisor is a four digit number, the numerical error
/ of using only the high words of both b and c to determine
/ the quotient are such that you get either a or a+1.
/ If you use the notation b(N) to mean the digit for 256**N position,
/ then this corresponds to the greatest integer in the quotient:
/	b(3)b(2)/c(3)c(2)
/ which is always >= the real quotient.  To see that it is never
/ too high by more than 1, consider the following:
/ First:
/	b(3)b(2)/c(3)c(2) = b(3)b(2)00/c(3)c(2)00
/ Then the error is:
/	b(3)b(2)00/c(3)c(2)00 - b(3)b(2)b(1)b(0)/c(3)c(2)c(1)c(0)
/ which can be simplified to:
/	b(3)b(2)00/c(3)c(2)c(1)c(0) * c(1)c(0)/c(3)c(2)00
/		- b(1)b(0)/c(3)c(2)c(1)c(0)
/ If we use M to the max digit for the radix in use, the largest
/ value of the positive (first) term is:
/	MM00/10MM * MM/1000 < .MM *.MM < 1
/ Since the difference under consideration was using the rational
/ values of the quotients rather than the greatest integer in them,
/ the greatest integer in b[HW]/c[HW] can be off by 1 on the high side
/ (but no more than 1).
/ A similar analysis is behind other situations covered below.
/ Those situations will only quote the results.
/
/ The full divide algorithm needs the original b and c values
/ and all of the registers, so the values of b and c must be saved.
/
	mov	%bx,SAVECLW
	mov	%cx,SAVECHW
	mov	%ax,SAVEBLW
	mov	%dx,SAVEBHW
/ Now check the high byte of c:
	andb	%ch,%ch
	jz	shftdiv		/ jump if its high byte is zero

/ c(3) != 0 and the above analysis applies, so do it.
	mov	%cx,%bx		/ set up c[HW]
	mov	%dx,%ax		/ and b[HW]
	xor	%dx,%dx		/ and clear high dividend bits
	jmp	dodiv

shftdiv:
/ c(3) == 0.
/ Shifting by bytes to zero the high word has to high of a
/ potential error: almost M.  More special cases could be
/ found, but we are getting to the point of diminishing returns.
/ for the general case that is left, ther radix must be 2
/ and shifts must be done minimally to get the potential error
/ down to 1.  Further, the only convenient shift below bytes
/ is single bits.  So the rest go through a bit shift loop.
/
	shr	%dx		/ low bit to carry
	rcr	%ax		/ carry to high bit
	shr	%cx		/ the same with the divisor
	rcr	%bx
	and	%cx,%cx		/ is c[HW] still nonzero?
	jnz	shftdiv

dodiv:
	div	%bx		/ %bx now the pseudo divisor
/ Now we must determine if we are off by 1.
	mov	%ax,%si		/ save pseudo-quotient (PQ)
	mov	SAVECHW,%ax	/ calc. c * PQ
				/ = c[LW] * PQ + 2**16 * c[HW] * PQ
	mul	%si		/ where * is unsigned.
	mov	%ax,%cx		/ save c[HW] * PQ
	mov	SAVECLW,%ax
	mul	%si		/ c[LW] * PQ
	add	%cx,%dx		/ finishes c * PQ
/ if c * PQ > b, we are off by 1:
	cmp	%dx,SAVEBHW	/ this is SAVEBHW - %dx
	jb	offby1
	ja	setqval
	cmp	%ax,SAVEBLW	/ this is SAVEBLW - %ax
	jae	setqval

offby1:
	test	LREMFLAG,%di	/ is remainder desired?
	jnz	roffby1		/ then go fix up product to get remainder
	dec	%si

setqval:
	test	LREMFLAG,%di	/ is remainder desired?
	jnz	setrval		/ then go calculate it
	mov	%si,%ax
	xor	%dx,%dx

/ all that is left is establishing the sign of the quotient

fixsign:
	and	%di,%di
	je	divret
	xor	BOTHFLAG,%di
	je	divret
	neg	%dx
	neg	%ax
	sbb	$0,%dx

divret:
	leave

#if LARGE_M | HUGE_M | COMPACT_M
	lret
#else
	ret
#endif

roffby1:			/ PQ was 1 too big, so fix product
	sub	SAVECLW,%ax
	sbb	SAVECHW,%dx
setrval:
	mov	%ax,%bx
	mov	%dx,%cx
	mov	SAVEBLW,%ax
	mov	SAVEBHW,%dx
	sub	%bx,%ax
	sbb	%cx,%dx
	and	DVNDFLAG,%di
	jmp	fixsign
