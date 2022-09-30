	.file	"strpbrk.s"
/	@(#)	1.1
/ Fast assembler language version of the following C-program
/			strpbrk
/ which represents the `standard' for the C-library.

/  Return ptr to first occurance of any character from `brkset'
/    in the character string `string'; NULL if none exists.

/	char *
/	strpbrk(string, brkset)
/	register char *string, *brkset;
/	{
/		register char *p;
/	
/		if(!string || !brkset)
/			return(0);
/		do {
/			for(p=brkset; *p != '\0' && *p != *string; ++p)
/				;
/			if(*p != '\0')
/				return(string);
/		}
/		while(*string++);
/		return(0);
/	}

/ al = string work area
/ bl = c
/ cx = segment length
/ dx = temp
/ ds,si = sp

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	strpbrk

strpbrk:
	enter	$256,$0		/ create space for translation table

	MCOUNT			/ call subroutine entry counter if profiling

	push	%di
	push	%si
	push	%bx
	push	%cx

/
/	create xlat translation table
/

	clr	%ax
	lea	-256(%bp),%di
	mov	%ss,%cx
	mov	%cx,%es
	mov	$128,%cx
	rep;ssto		/ initialise table to all zeroes

#if LARGE_M | HUGE_M | COMPACT_M
	lds	PARAM(3),%si	/ ds,si = brkset
#else
	mov	PARAM(2),%si	/ ds,si = brkset
#endif
	movb	$255,%bl	/ character to set in table so that sets non
				/ zero and sign flag when tested

setc1:
	mov	%si,%cx		/ set cx to max count before segment overflow
	neg	%cx
	jnz	lp1
	dec	%cx
lp1:
	slodb			/ *sp++ ==   NOTE %ah == 0
	mov	%ax,%di		/ where it can be used as an index
	movb	%bl,-256(%bp,%di)/ set matching char to non-zero
	testb	%al		/ end of brkset?
	loopnz	lp1		/ loop until end of brkset or count == 0
	jz	endofbrkset	/ skip if end of brkset
	test	%si		/ end of segment?
	jnz	setc1		/ if not, go back and try again
#if HUGE_M
	mov	%ds,%dx		/ reset for next segment
	add	$8,%dx
	mov	%dx,%ds
	jmp	setc1		/ continue in next segment
#else
	jmp	fault		/ segment overflow, generate fault
#endif

/
/ now have xlat translation table ready on stack
/ get chars from string and translate them
/



endofbrkset:
#if LARGE_M | HUGE_M | COMPACT_M
	lds	PARAM(1),%si	/ ds,si = string
#else
	mov	PARAM(1),%si	/ ds,si = string
#endif

	lea	-256(%bp),%bx	/ set bx to point to xlat table
	movb	$1,%ss:(%bx)	/ set entry 0 to 1
				/ can now distinguish between end of string
				/ and character in brkset
setc2:
	mov	%si,%cx		/ set cx to max count before segment overflow
	neg	%cx
	jnz	lp2
	dec	%cx
lp2:
	slodb			/ *string++
#if HUGE_M | LARGE_M | COMPACT_M
/
/	xlat	%ss:(%bx)
/	but this assembler won't allow a segment override
/ so ....
	.byte	0x36		/ %ss segment override prefix
#endif
	xlat
	testb	%al
	loopz	lp2		/ loop until character in brkset 
				/ or count == 0
	jnz	endoftest	/ either   character in brkset - sign flag
				/ or	   end of string - no sign flag
	test	%si		/ end of segment?
	jnz	setc2		/ if not, go back and try again
#if HUGE_M
	mov	%ds,%dx		/ reset for next segment
	add	$8,%dx
	mov	%dx,%ds
	jmp	setc2		/ continue in next segment
#else
fault:
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif

/
/ end of test - return pointer to last char
/ ds,si points to one past last character 
/
/ sign bit set - character in brkset
/ sign bit clr - end of string

endoftest:
	jns	retnull
	mov	%si,%ax
	dec	%ax
#if LARGE_M | HUGE_M | COMPACT_M
	mov	%ds,%dx
#endif
	jmp	ret
retnull:
	clr	%ax
#if LARGE_M | HUGE_M | COMPACT_M
	clr	%dx
#endif
ret:
	pop	%cx
	pop	%bx
	pop	%si
	pop	%di

	LVRET
