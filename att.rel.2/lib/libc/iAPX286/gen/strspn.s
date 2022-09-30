	.file	"strspn.s"
/	@(#)	1.1
/ Fast assembler language version of the following C-program
/			strspn
/ which represents the `standard' for the C-library.

/	Return the number of characters in the maximum leading segment
/	of string which consists solely of characters from charset.

/	int
/	strspn(string, charset)
/	char	*string;
/	register char	*charset;
/	{
/		register char *p, *q;
/	
/		for(q=string; *q != '\0'; ++q) {
/			for(p=charset; *p != '\0' && *p != *q; ++p)
/				;
/			if(*p == '\0')
/				break;
/		}
/		return(q-string);
/	}

/ al = string work area
/ bl = c
/ cx = segment length
/ dx = temp
/ ds,si = sp

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	strspn

strspn:
	enter	$256,$0		/ create space for translation table

	MCOUNT			/ call subroutine entry counter if profiling

	push	%di
	push	%si
	push	%bx
	push	%cx

/
/	create xlat translation table
/

	clr	%ax		/ ax = 0
	lea	-256(%bp),%di
	mov	%ss,%cx
	mov	%cx,%es
	mov	$128,%cx
	rep;ssto		/ initialise table to zeros

#if LARGE_M | HUGE_M | COMPACT_M
	lds	PARAM(3),%si	/ ds,si = charset
#else
	mov	PARAM(2),%si	/ ds,si = charset
#endif
	movb	$255,%bl	/ character to set in table so that sets sign
				/ flag when tested
setc1:
	mov	%si,%cx		/ set cx to max count before segment overflow
	neg	%cx
	jnz	lp1
	dec	%cx
lp1:
	slodb			/ *sp++ ==   NOTE %ah == 0
	mov	%ax,%di		/ where it can be used as an index
	movb	%bl,-256(%bp,%di)/ set matching char to non-zero
	testb	%al		/ end of charset?
	loopnz	lp1		/ loop until end of charset or count == 0
	jz	endofcharset	/ skip if end of charset
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



endofcharset:
#if LARGE_M | HUGE_M | COMPACT_M
	lds	PARAM(1),%si	/ ds,si = string
#else
	mov	PARAM(1),%si	/ ds,si = string
#endif

	lea	-256(%bp),%bx	/ set bx to point to xlat table
	movb	$0,%ss:(%bx)	/ set entry 0 to 0
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
	loopnz	lp2		/ loop until character not in charset 
				/ or count == 0
	jz	endoftest	/ either   character not in charset
				/ or	   end of string
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
/ end of test - return number of matched chars
/ ds,si points to one past last character 
/

endoftest:
	mov	%si,%ax
	sub	PARAM(1),%ax
	dec	%ax

	pop	%cx
	pop	%bx
	pop	%si
	pop	%di

	LVRET
