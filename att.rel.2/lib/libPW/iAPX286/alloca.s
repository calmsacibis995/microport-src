	.file	"alloca.s"
/	@(#)	1.2
	.text
	.def    alloca; .val alloca; .scl 2; .type 044; .endef
	.globl  alloca
alloca:
#if LARGE_M
	pop	%si
#endif
	pop	%dx
	pop	%ax
	inc	%ax
	and	$0xfffe,%ax
	sub	%ax,%sp
	mov	%sp,%ax
	push	%ax
	push	%dx
#if LARGE_M
	push	%si
	mov	%ss,%dx
	lret
#else
	ret
#endif
       .def    alloca;   .val    .;      .scl    -1;     .endef
	.set	.T1,1
        .set    .S1,0
	.set	.F1,0
