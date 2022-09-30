	.file	"mcount.s"
/	@(#)	1.1
	.text
/
/ This routine is called at the entry to each subroutine compiled with the
/ -p option to cc(1).
/ It expects %si to contain the address of a ptr to a long.  The ptr must be
/ initialized to 0.  (Except in huge model, the ptr must just be the 16 bit
/ offset.)  In compact, large, and huge models, %bx is expected to contain
/ the segment corresponding to the address in %si.
/
/	struct counter {
/		void	(*addr)();	/* address of calling routine */
/		long	count;		/* # of times called by caller */
/	} *_countbase;
/
/	It is assumed that the "addr" field is always being referenced by
/	a large model program no matter what model is actually running.

	.globl	_mcount
_mcount:
#if SMALL_M
	pop	%dx		/ %dx = return address to caller
	push	%bx		/ save regs
	mov	(%si),%bx	/ get ptr to counter
	test	%bx		/ if not 0, the caller called before:
	jnz	incr		/	skip initialization
	mov	_countbase,%bx	/ get address of next counter structure location
	test	%bx		/ if NULL, counting has been disabled:
	jz	ret		/	get out
	add	$8,_countbase	/ reset ptr to next counter structure
	mov	%dx,(%bx)	/ set addr in counter structure
	mov	%cs,2(%bx)
	add	$4,%bx		/ set up address of count field
	mov	%bx,(%si)	/	and store it in caller's ptr
incr:
	add	$1,(%bx)	/ increment the count field
	adc	$0,2(%bx)
ret:
	mov	spmax,%ax	/ update max stack
	cmp	%sp,%ax
	jbe	.stk
	mov	%sp,%ax
	mov	%ax,spmax
.stk:
	pop	%bx		/ restore regs
	push	%dx		/ restore caller's return address
	ret
#endif
#if LARGE_M || HUGE_M
	mov	%bx,%ds			/ get access to ptr
	mov	$<s>_countbase,%ax	/	and _countbase
	mov	%ax,%es
	test	(%si)			/ if ptr != 0, the caller called before:
	jnz	incr			/	skip initialization
	test	%es:_countbase+2	/ if 0, counting has been disabled:
	jz	ret			/	get out
	les	%es:_countbase,%di	/ set ptr to address of count field
	add	$4,%di
#if HUGE_M
	jnc	ok			/ check for segment switch
	mov	%ax,%es			/ for the time being, disable counting
	mov	$0,%es:_countbase+2
	jmp	ret			/ and get out
ok:
#endif
	mov	%di,(%si)
	pop	%ax			/ get caller's address
	pop	%bx
	mov	%ax,%es:-4(%di)		/ save caller's address in addr
	mov	%bx,%es:-2(%di)
	push	%bx			/ restore caller's address for return
	push	%ax
	mov	$<s>_countbase,%ax	/ _countbase++
	mov	%ax,%es
	add	$8,%es:_countbase
incr:
	mov	%es:_countbase+2,%ax	/ get access to count
	mov	%ax,%es
	mov	(%si),%si
	add	$1,%es:(%si)		/ count++
	adc	$0,%es:2(%si)
ret:
	mov	$<s>spmax,%ax
	mov	%ax,%ds
	mov	%ds:spmax,%ax
	cmp	%sp,%ax
	jbe	.stk
	mov	%sp,%ax
	mov	%ax,%ds:spmax
.stk:
	lret
#endif
#if MIDDLE_M | COMPACT_M
	XX	%XX			/ generate syntax error since available
					/ registers are not yet known and we
					/ can't write it yet for these models
#endif
