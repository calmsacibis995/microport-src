	 .file "xtrans.s"
/	@(#)	1.1

/*

	iAPX286 @(#)xtrans.s	1.3 - 85/06/24

	80287 Math Library Transcendental Functions

	      cosine, sine, tangent


	Global Functions:

		xcos (cosine)
		xsin (sine)
		xtan (tangent)


*/

#define	M_PI_4 		0.78539816339744830962
#define INDEFINITE	0xffc00000

/* condition codes							*/
#define COND0		0x01
#define COND1		0x02	
#define COND2		0x04
#define COND3		0x40
#define NCOND3		0xfd
#define C3nC1		0x42

	.data
	.even

	.globl	m_pi_4
	.globl	indefinite

m_pi_4:		.double	M_PI_4
indefinite:	.long	INDEFINITE

	.even
	.text
	.globl xsin
	.globl xcos
	.globl xtan


/************************************************************************/
/*	cosine function							*/
/************************************************************************/
xcos:
	enter	$0,$0
	push	%ax
	push	%cx
	push	%bx
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	fldl	6(%bp)		/ load angle argument into TOS
#else
	fldl	4(%bp)		/ load angle argument into TOS
#endif
	fxam			/ look at TOS argument
	fstsw	%ax		/ save status in ax
	fldl	m_pi_4		/ setup for angle reduction
	movb	$1,%cl		/ flag cosine function, internal use
	sahf			/ store ah into flags
	jc	ill_parm

	fxch			/ swap TOS elements
	jpe	its_sine	/ jump if normal or denormal
	
	fstp	%st(1)		/ pop m_pi_4
	jnz	sine_normalize

	fstp	%st(0)		/ remove 0
	fld1			/ load +1.0 on TOS
	jmp	return_to_caller

sine0_unnormal:
	fstp	%st(1)				/ remove PI/4
	jnz	sine_normalize
	jmp	return_to_caller

sine_normalize:
	call	normalize_arg
	jmp	its_sine

/************************************************************************/
/*	sine function							*/
/************************************************************************/
xsin:
	enter	$0,$0
	push	%ax
	push	%cx
	push	%bx
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	fldl	6(%bp)		/ load angle argument into TOS
#else
	fldl	4(%bp)		/ load angle argument into TOS
#endif
	fxam			/ look at TOS argument
	fstsw	%ax		/ save status in ax
	fldl	m_pi_4		/ setup for angle reduction
	sahf			/ store ah into flags
	jc	ill_parm

	fxch			/ swap TOS elements
	movb	$0,%cl		/ flag sine function, internal use
	jpo	sine0_unnormal

its_sine:
	fprem			/ perform angle reduction
	xchg	%bx,%ax		/ save status in bx
	fstsw	%ax		/ get argument status into ax
	xchg	%bx,%ax		/ put new status into bx
	testb	$COND2,%bh
	jnz	out_of_range

	fabs			/ force to positive
	orb	%cl,%cl		/ test for sine or cos function
	jz	sine_select	/ it is a sine request
	
	andb	$NCOND3,%ah	/ turn off sign
	orb	$0x80,%bh
	addb	$COND3,%bh	/ position into carry flag
	movb	$0,%al		/ extract carry flag
	rclb	$1,%al
	xorb	%al,%bh		/ add carry, dont change c1 flag

sine_select:
	testb	$COND1,%bh	/ reverse angle if true
	jz	no_sine_rotate

	fsub			/ invert 
	fabs/
	jmp	sine_fptan	/ angle is now within range:
				/ 0 < argument <= M_PI_4
no_sine_rotate:
	ftst			/ test for 0 angle
	xchg	%cx,%ax		/ save ax	
	fstsw	%ax		/ c3 = 1 if TOS = 0
	xchg	%cx,%ax		/ restore ax
	fstp	%st(1)		/ pop M_PI_4
	testb	$COND3,%ch	/ arg == 0 if c3 == 1
	jnz	sine_argument_0

sine_fptan:
	fptan			/ TAN TOS = %st(1)/%st(0) = x/y

post_sine_fptan:
	testb	$C3nC1,%bh / determine octant angle
	jpo	x_as_numerator
	
	fld	%st(1)		/ copy y value
	jmp	wrap_sine

ill_parm:
	fstp	%st(0)		/ pop M_PI_4
	jz	return_empty	/ no parm
	jpo	return_nan	/ value is NAN
	fprem
return_nan:
return_empty:
	jmp	return_to_caller

sine_argument_0:
	fld1			/ simulate fptan with TOS == 0
	jmp	post_sine_fptan

out_of_range:			/ angle is too big
	fcompp			/ pop TOS twice
	fildl	indefinite	/ return indefinite value
	jmp 	return_to_caller

x_as_numerator:
	fld	%st(0)		/ copy x value
	fxch	%st(2)		/ put x in numerator

wrap_sine:
	fmul	%st(0),%st	/ form x*x + y*y
	fxch
	fmul	%st(0),%st	
	fadd			/ answer is in TOS
	fsqrt			/ sqrt(TOS)
	fxch			/ swap args, get ready for divide

	andb	$COND0,%bh	/ fprem c0 flag
	andb	$COND1,%ah	/ fxam  c1 flag
	orb	%ah,%bh		/ even number of flags cancel
	jpe	positive_sine	/ two negatives make a positive
	fchs			/ force to negative on 80287 stack

positive_sine:
	fdiv			/ form final result

return_to_caller:
	pop	%bx
	pop	%cx
	pop	%ax

	leave
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	lret
#else
	ret
#endif

/************************************************************************/
/*	tangent function						*/
/************************************************************************/
xtan:
	enter	$0,$0
	push	%ax
	push	%cx
	push	%bx
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	fldl	6(%bp)		/ load angle argument into TOS
#else
	fldl	4(%bp)		/ load angle argument into TOS
#endif
	fxam			/ look at TOS argument
	fstsw	%ax		/ save status in ax
	fldl	m_pi_4		/ setup for angle reduction
	sahf			/ store ah into flags
	jc	ill_parm

	fxch			/ swap TOS elements
	jpe	tan_normal

tan_0_unnormal:
	fstp	%st(1)		/ pop m_pi_4
	jz	tan_angle_0	/ jump if angle is 0
	call	normalize_arg

tan_normal:
	fprem			/ angle reduction
	xchg	%bx,%ax		/save status in bx
	fstsw	%ax
	xchg	%bx,%ax		/ restore status
	testb	$COND2,%bh	/ test for complete reduction
	jnz	out_of_range
	fabs			/force positive
	testb	$COND1,%bh
	jz	no_tan_reverse
	fsub			/ reverse angle
	jmp	do_fptan

tan_angle_0:
	jmp	return_to_caller

no_tan_reverse:
	ftst			/ test for zero angle
	xchg	%cx,%ax		/ save status
	fstsw	%ax		/ c3 == 1 if TOS == 0
	xchg	%cx,%ax
	fstp	%st(1)		/ pop m_p1_4
	testb	$COND3,%ch
	jnz	tan_0

do_fptan:
	fptan			/ tan TOS = st(1)/st(0)

after_fptan:
	movb	%bh,%al		/ get copy of fprem c3 flag
	andb 	$0x42,%al
	testb	$C3nC1,%bh

	jpo	reverse_divide
	orb	%ah,%al
	jpe	positive_divide
	fchs			/ force result negative on 80287

positive_divide:
	fdivr
	jmp	return_to_caller

tan_0:
	fld1
	jmp	after_fptan

reverse_divide:
	orb	%ah,%al
	jpe	positive_r_divide
	fchs			/ force result negative on 80287

positive_r_divide:
	fdiv			/ form reciprocal of result
	jmp	return_to_caller

normalize_arg:
	fabs			/ force positive value
	fxtract			/ 0 <= st(0) < 1
	fld1			/ get normal bit
	fadd	%st,%st(1)	/ normalize fraction
	fsub			/ restore original vaue
	fscale			/ form original normalized value
	fstp	%st(1)		/ remove scale factor
	fldl	m_pi_4		/ get m_pi_4
	fxch			/ swap TOS
	fwait
	ret

