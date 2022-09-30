	.file	"dbxxx.s"
/	@(#)	1.2
	.data
	.globl	__dbargs
__dbargs:
	.=.+512
	.text
	.globl	__dbsubc
__dbsubc:
#if LARGE_M | HUGE_M | COMPACT_M
	mov	$<s>__dbargs,%ax
	mov	%ax,%ds
#endif
#if SMALL_M
	mov	$0x5f,%ax
	mov	%ax,%ds
#endif
	mov	$__dbargs,%si
	mov	4(%si),%cx		/ number of argument words
	mov	%cx,%bx			/ convert to byte offset in array
	shl	%bx
	add	$6,%bx			/ skip the address and the size
looplb:
	sub	$2,%bx
	push	(%bx,%si)
	loop	looplb
#if LARGE_M | HUGE_M | COMPACT_M
	lcall	*(%si)
#else
	call	*(%si)
#endif
	.globl	__dbsubn
__dbsubn:
	int	$3
	.data
