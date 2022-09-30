	.file	"sbrk.s"
/	@(#)	1.1
/
/ char *sbrk(incr);
/ int	incr;

#include "lib.s.h"
#define	BRKCODE		0
#define	SBRKCODE	1

#if SMALL_M
/
/	Keep track of actual end of memory here
/	The kernel allocates and de-allocates only in units of clicks 
/	(512 bytes) so we need to do this here
/
	.data
brk_end_data:
	.value	end
#endif

	.text

	.globl	brk
	.globl	sbrk

sbrk:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if SMALL_M
	test	PARAM(1)	/ check if zero
	jnz	change_sbrk
	mov	brk_end_data,%ax	/ return current limit
	LVRET

/
/	We do the handling of sub-click granularity here
/

change_sbrk:
	mov	PARAM(1),%ax
	cwd				/ sign extend 
	add	brk_end_data,%ax
	adc	$0,%dx
	jz	change_ok
	JMP	_cerror
change_ok:
	push	%ds
	push	%ax
	push	$BRKCODE
	push	$BRK
	lcall	SYSCALL
	jnc	noerror
	JMP	_cerror
noerror:
	mov	brk_end_data,%ax
	mov	-4(%bp),%bx	/ cheat - really first param passed to BRK above
	mov	%bx,brk_end_data
	LVRET
#else
	push	PARAM(1)	/ incr
	push	$SBRKCODE	/ function code for sbreak sys call
	push	$BRK
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller

#endif

/
/ int brk(endds);
/ char	*endds;

brk:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(2)	/ <s>endds
#else
	push	%ds
#endif
	push	PARAM(1)	/ endds
	push	$BRKCODE	/ function code for sbreak sys call
	push	$BRK
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror2
	JMP	_cerror
noerror2:
#if SMALL_M
	mov	PARAM(1),%ax	
	mov	%ax,brk_end_data	/ update this record
#endif
	xor	%ax,%ax		/ set return code = 0 on success
	LVRET			/ restore stack frame and return to caller
