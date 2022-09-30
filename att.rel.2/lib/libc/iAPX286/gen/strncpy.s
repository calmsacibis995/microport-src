	.file	"strncpy.s"
/	@(#)	1.1
/ Fast assembler language version of the following C-program
/			strncpy
/ which represents the `standard' for the C-library.

/ Copy s2 to s1, truncating or null-padding to always copy n bytes.
/ Return s1

/	char *
/	strncpy(s1, s2, n)
/	register char *s1, *s2;
/	register int n;
/	{
/		register char *os1 = s1;
/	
/		while (--n >= 0)
/			if ((*s1++ = *s2++) == '\0')
/				while (--n >= 0)
/					*s1++ = '\0';
/		return (os1);
/	}
/ ax = copy work area
/ bl = c
/ cx = n
/ dx = temp
/ ds,si = s2
/ es,di = s1

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	strncpy

strncpy:
	enter	$2,$0		/ reserve space for n_left
	MCOUNT			/ call subroutine entry counter if profiling
	push	%bx		/ save registers
	push	%cx
	push	%di
#if LARGE_M | HUGE_M | COMPACT_M
	les	PARAM(1),%di	/ es,di = s1
	lds	PARAM(3),%si	/ ds,si = s2
	mov	PARAM(5),%cx	/ cx = n
#else
	mov	PARAM(1),%di	/ es,di = s1
	mov	%ds,%dx
	mov	%dx,%es
	mov	PARAM(2),%si	/ ds,si = s2
	mov	PARAM(3),%cx	/ cx = n
#endif
	test	%cx		/ verify that n > 0
	jle	ret		/	skip if not
setn:
	mov	$0,-2(%bp)	/ initialize n_left after this time thru loop
	mov	%di,%dx		/ check for s1 segment overflow before n == 0
	neg	%dx
	jz	s1ok		/ skip if at start of segment
	cmp	%cx,%dx
	jae	s1ok		/	or remainder of segment >= n
	add	%cx,-2(%bp)	/ reset n_left and n
	sub	%dx,-2(%bp)
	mov	%dx,%cx
s1ok:
	mov	%si,%dx		/ check for s2 segment overflow before n == 0
	neg	%dx
	jz	lp1		/ skip if at start of segment
	cmp	%cx,%dx
	jae	lp1		/	or remainder of segment >= n
	add	%cx,-2(%bp)	/ reset n_left and n
	sub	%dx,-2(%bp)
	mov	%dx,%cx
lp1:
	slodb			/ get next character from s2
	sstob			/ put it in s1
	cmpb	$0,%al		/ copied char == '\0'
	loopne	lp1		/ loop until copied char == '\0' or n == 0
	je	copiednull	/ skip if copied char == '\0'
	test	-2(%bp)		/ done?
	jz	ret		/	skip if yes
#if HUGE_M
	mov	-2(%bp),%cx	/ set n to n_left
	test	%si		/ check for s2 break into new segment
	jnz	srcok		/	skip if not
	mov	%ds,%dx		/ reset ds for next segment
	add	$8,%dx
	mov	%dx,%ds
srcok:
	test	%di		/ check for s1 break into new segment
	jnz	setn		/	skip if not
	mov	%es,%dx		/ reset es for next segment
	add	$8,%dx
	mov	$dx,%es
	jmp	setn		/ continue in next segment
#else
killit:
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
ret:
	mov	PARAM(1),%ax	/ return original s1
#if LARGE_M | HUGE_M | COMPACT_M
	mov	PARAM(2),%dx
#endif
	pop	%di		/ restore registers
	pop	%cx
	pop	%bx
	LVRET			/ restore stack frame and return to caller
/
/	copied null charcter
/		now need to pad rest of destination with nulls
/
/	%cx + -2(%bp) = number of nulls to pad
/	%es:di	next position to add padding
/
copiednull:
	test	%cx		/ are there more in %cx
	jnz	lp2		/ do them
restart1:
	test	-2(%bp)		/ any more there
	jz	ret		/ no - we're all done
	mov	-2(%bp),%cx	/ cx now is number of chars left to do
	mov	$0,-2(%bp)	/ and set -2(%bp) as though all done
	test	%di		/ check for break into new segment
#if HUGE_M
	jnz	destok
	mov	%es,%dx		/ update segement
	add	$8,%dx
	mov	%dx,%es
#else
	jz	killit		/ we're at the end of the segment
#endif
destok:
	mov	%di,%dx		/ check for segment overflow before n == 0
	neg	%dx
	jz	lp2		/ ok if at start of segment
	cmp	%cx,%dx
	jae	lp2		/ ok - remainder of segment > n
	sub	%dx,%cx
	mov	%cx,-2(%bp)	/ -2(%bp) = how many we can't do
	mov	%dx,%cx		/ cx = how many we can

lp2:
	rep ; sstob		/ al still contains null
	jmp	restart1
