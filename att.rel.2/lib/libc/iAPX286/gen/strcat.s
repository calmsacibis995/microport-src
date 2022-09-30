	.file	"strcat.s"
/	@(#)	1.1

/ Concatenate s2 on the end of s1.  S1's space must be large enough.
/ Return s1.

/	char *
/	strcat(s1, s2)
/	register char *s1, *s2;
/	{
/		register char *os1;
/	
/		os1 = s1;
/		while (*s1++)
/			;
/		--s1;
/		while (*s1++ = *s2++)
/			;
/		return(os1);
/	}

/ al = string work area
/ cx = segment length
/ dx = temp
/ ds,si = s2
/ es,di = s1

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	strcat

strcat:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling
	push	%cx		/ save registers
	push	%di
#if LARGE_M | HUGE_M | COMPACT_M
	les	PARAM(1),%di	/ es,di = s1
	lds	PARAM(3),%si	/ ds,si = s2
#else
	mov	PARAM(1),%di	/ es,di = s1
	mov	%ds,%dx
	mov	%dx,%es
	mov	PARAM(2),%si	/ ds,si = s2
#endif
	clrb	%al		/ set al = \0 for scan for end of s1
set1:
	mov	%di,%cx		/ set cx to # bytes left in segment for scan cnt
	neg	%cx
	jnz	lp1		/ skip if count OK
	dec	%cx		/ set cx to 65535
lp1:
	repnz ;	scab		/ loop to end of s1 or count == 0
	jz	copy		/ skip if end of s1
	test	%di		/ end of segment?
	jnz	set1		/ if not, go back and try again
#if HUGE_M
	mov	%es,%dx		/ reset to continue in next segment
	add	$8,%dx
	mov	%dx,%es
	jmp	set1		/ go back for next segment
#else
	jmp	overflow	/ segment overflow, generate fault
#endif
copy:
	dec	%di		/ s1-- to copy over the null
set2:
	mov	%di,%cx		/ set cx to largest count possible without
	neg	%cx		/	segment overflow
	jnz	set3
	dec	%cx
set3:
	mov	%si,%dx		/ see if s2 will cross segment before s1
	neg	%dx
	jz	lp2		/ skip if s2 is at start of segment
	cmp	%cx,%dx
	jae	lp2		/ skip if count is OK
	mov	%dx,%cx		/ reset count
lp2:
	slodb			/ get s2++
	sstob			/ put it at s1++
	testb	%al		/ end of string?
	loopne	lp2		/ loop until end of string or count == 0
	jz	ret		/ skip if end of string copied
	test	%di		/ end of segment for s1?
#if HUGE_M
	jnz	s1ok		/	skip if not
	mov	%es,%dx		/ reset s1 for next segment
	add	$8,%dx
	mov	%dx,%es
s1ok:
#else
	jz	overflow	/ jmp if end of segment
#endif
	test	%si		/ end of segment for s2?
	jnz	set2		/	continue copy if not
#if HUGE_M
	mov	%ds,%dx		/ reset s2 for next segment
	add	$8,%dx
	mov	%dx,%es
	jmp	set2		/ continue copy
#else
overflow:
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
	LVRET			/ restore stack frame and return to caller
