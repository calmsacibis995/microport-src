	.file	"strncat.s"
/	@(#)	1.2
/ Fast assembler language version of the following C-program
/			strncat
/ which represents the `standard' for the C-library.

/ Concatenate s2 on the end of s1.  S1's space must be large enough.
/ At most n characters are moved.  Return (null terminated) s1.

/	char *
/	strncat(s1, s2, n)
/	register char *s1, *s2;
/	register n;
/	{
/		register char *os1;
/	
/		os1 = s1;
/		while(*s1++)
/			;
/		--s1;
/		while(*s1++ = *s2++)
/			if(--n < 0) {
/				*--s1 = '\0';
/				break;
/			}
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
	.globl	strncat

strncat:
	enter	$2,$0		/ reserve space for n_left
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
/
/	do the copy (cf. strncpy )
/
copy:
	dec	%di		/ s1-- to copy over the null
#if HUGE_M | LARGE_M | COMPACT_M
	mov	PARAM(5),%cx	/ n
#else
	mov	PARAM(3),%cx	/ n
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
	jz	lp2		/ skip if at start of segment
	cmp	%cx,%dx
	jae	lp2		/	or remainder of segment >= n
	add	%cx,-2(%bp)	/ reset n_left and n
	sub	%dx,-2(%bp)
	mov	%dx,%cx
lp2:
	slodb			/ get next character from s2
	sstob			/ put it in s1
	cmpb	$0,%al		/ copied char == '\0'
	loopne	lp2		/ loop until copied char == '\0' or n == 0
	je	ret		/ skip if copied char == '\0'
	test	-2(%bp)		/ done?
	jz	ret1		/	skip if yes
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
overflow:
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
ret1:				/ copy stopped after n - need to append null
				/ es:di = next char
	test	%di		/ check for s1 break into new segment
#if HUGE_M
	jnz	setnull
	mov	%es,%dx		/ reset es for next segment
	add	$8,%dx
	mov	$dx,%es
setnull:
	movb	$0,%es:(%di)	/ append null
#else
	jz	overflow	/ error
#if LARGE_M || COMPACT_M
	movb	$0,%es:(%di)	/ append null
#else
	movb	$0,(%di)	/ append null
#endif
#endif
ret:
	mov	PARAM(1),%ax	/ return original s1
#if	HUGE_M | LARGE_M | COMPACT_M
	mov	PARAM(2),%dx
#endif
	pop	%di
	pop	%cx		/ reset registers
	LVRET			/ restore stack frame and return to caller
