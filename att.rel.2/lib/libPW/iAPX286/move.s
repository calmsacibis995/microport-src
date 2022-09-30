	.file	"move.s"
/	@(#)	1.1
	.text
	.def    _move; .val _move; .scl 2; .type 044; .endef
	.globl  _move
_move:
	enter	$0,$0
	push	%cx
	push	%si
	push	%di
#if SMALL_M
#define RET ret
	mov	4(%bp),%si
	mov	6(%bp),%di
	mov	8(%bp),%cx
#endif
#if LARGE_M
#define RET lret
	mov	8(%bp),%si
	mov	%si,%ds
	mov	6(%bp),%si
	mov	12(%bp),%di
	mov	%di,%es
	mov	10(%bp),%es
	mov	14(%bp),%cx
#endif
	repz	
	smovb
	pop	%di
	pop	%si
	pop	%cx
	leave
	RET
       .def    _move;   .val    .;      .scl    -1;     .endef
	.data
