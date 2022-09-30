	.file	"userio.s"
/ uportid = "@(#)userio.s	Microport Rev Id  1.3.3 6/18/86";
/
/ @(#)userio.s	1.19
/	Various routines to get stuff to and from the user.
/	Also, routines to get and put physical addresses.
/

#include	"sys/mmu.h"

#define	READ	1
#define	WRITE	0

	.text

/
/ fubyte	fetch user byte
/ fuword	fetch user word
/ subyte	store user byte
/ suword	store user word
/ suiword	store user i-space word
/ fuiword	fetch user i-space word
/
/	fu{byte/[i]word}( pointer )
/	su{byte/[i]word}( pointer, value )
/
/	stores return zero on success; -1 on failure
/	fetches return desired value on success; -1 on failure
/
/ NOTE: in this implementation, fuiword is the same as fuword,
/	and suiword is the same as suword
/
	.globl	fubyte
	.globl	fuword
	.globl	fuiword
	.globl	subyte
	.globl	suword
	.globl	suiword
/
/ FUBYTE
/
fubyte:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%di			/ save what we gonna trash
/
/	check if the user can access the given address
/
	push	$READ			/ read access
	push	$1			/ one byte
	push	8(%bp)			/ selector
	push	6(%bp)			/ offset
	lcall	<s>useracc,useracc	/ check access
	add	$8,%sp			/ fix up stack
	or	%ax,%ax			/ ok to access?
	jz	nono			/ no, error and exit
	mov	8(%bp),%es		/ FINALLY, load the selector
	mov	6(%bp),%di		/ and the offset
	movb	%es:0(%di),%al		/ AND get the byte
	xorb	%ah,%ah			/ clear upper byte
	pop	%di
	pop	%bp
	lret

/
/ FUWORD, FUIWORD
/
fuword:
fuiword:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%di			/ save what we gonna trash
/
/	check if the user can access the given address
/
	push	$READ			/ read access
	push	$2			/ one word
	push	8(%bp)			/ selector
	push	6(%bp)			/ offset
	lcall	<s>useracc,useracc	/ check access
	add	$8,%sp			/ fix up stack
	or	%ax,%ax			/ ok to access?
	jz	nono			/ no, error and exit
	mov	8(%bp),%es		/ FINALLY, load the selector
	mov	6(%bp),%di		/ and the offset
	mov	%es:0(%di),%ax		/ AND get the word
	pop	%di			/ restore regs
	pop	%bp
	lret

/
/ SUBYTE
/
subyte:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%di			/ save what we gonna trash
/
/	check if the user can access the given address
/
	push	$WRITE			/ read access
	push	$1			/ one byte
	push	8(%bp)			/ selector
	push	6(%bp)			/ offset
	lcall	<s>useracc,useracc	/ check access
	add	$8,%sp			/ fix up stack
	or	%ax,%ax			/ ok to access?
	jz	nono			/ no, error and exit
	mov	8(%bp),%es		/ FINALLY, load the selector
	mov	6(%bp),%di		/ and the offset
	mov	10(%bp),%ax		/ get the value we want to store
	movb	%al,%es:0(%di)		/ AND store the byte
	xor	%ax,%ax			/ zero return code
	pop	%di			/ restore regs
	pop	%bp
	lret

/
/ SUWORD, SUIWORD
/
suword:
suiword:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%di			/ save what we gonna trash
/
/	check if the user can access the given address
/
	push	$WRITE			/ read access
	push	$2			/ one word
	push	8(%bp)			/ selector
	push	6(%bp)			/ offset
	lcall	<s>useracc,useracc	/ check access
	add	$8,%sp			/ fix up stack
	or	%ax,%ax			/ ok to access?
	jz	nono			/ no, error and exit
	mov	8(%bp),%es		/ FINALLY, load the selector
	mov	6(%bp),%di		/ and the offset
	mov	10(%bp),%ax		/ get the word we're gonna store
	mov	%ax,%es:0(%di)		/ AND store the word
	xor	%ax,%ax			/ zero return code
	pop	%di			/ restore regs
	pop	%bp
	lret

nono:
	mov	$-1,%ax			/ bad return value
	pop	%di
	pop	%bp
	lret

/
/ fpbyte, spbyte, fpword, spword
/	routines to fetch and store physical addresses
/
/	fp{byte/word}( physical_address )
/	sp{byte/word}( physical_address, value )
/
/ NOTE: you must push di to use mapit
/
	.globl	fpbyte
	.globl	fpword
	.globl	spbyte
	.globl	spword
fpbyte:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%bx			/ save stuff we are gonna trash
	push	%cx
	push	%di
	mov	$2,%bx			/ limit of seg desc
	mov	8(%bp),%dx		/ hi base of seg desc
	mov	6(%bp),%cx		/ low base of seg desc
	movb	$ACC_KDATA,%dh		/ r/w data access
	call	mapit			/ point es to this segment
	movb	%es:0(%di),%al		/ get the byte
	xorb	%ah,%ah 		/ clear upper byte
	pop	%di			/ restore registers
	pop	%cx
	pop	%bx
	pop	%bp
	lret

fpword:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%bx			/ save stuff we are gonna trash
	push	%cx
	push	%di
	mov	$2,%bx			/ limit of seg desc
	mov	8(%bp),%dx		/ hi base of seg desc
	mov	6(%bp),%cx		/ low base of seg desc
	movb	$ACC_KDATA,%dh		/ r/w data access
	call	mapit			/ point es to this segment
	mov	%es:0(%di),%ax		/ get the word
	pop	%di			/ restore registers
	pop	%cx
	pop	%bx
	pop	%bp
	lret

spbyte:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%bx			/ save stuff we are gonna trash
	push	%cx
	push	%di
	mov	$2,%bx			/ limit of seg desc
	mov	8(%bp),%dx		/ hi base of seg desc
	mov	6(%bp),%cx		/ low base of seg desc
	mov	10(%bp),%ax		/ byte to write
	movb	$ACC_KDATA,%dh		/ r/w data access
	call	mapit			/ point es to this segment
	movb	%al,%es:0(%di)		/ put the byte
	pop	%di			/ restore registers
	pop	%cx
	pop	%bx
	pop	%bp
	lret

spword:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%bx			/ save stuff we are gonna trash
	push	%cx
	push	%di
	mov	$2,%bx			/ limit of seg desc
	mov	8(%bp),%dx		/ hi base of seg desc
	mov	6(%bp),%cx		/ low base of seg desc
	mov	10(%bp),%ax		/ word to write
	movb	$ACC_KDATA,%dh		/ r/w data access
	call	mapit			/ point es to this segment
	mov	%ax,%es:0(%di)		/ put the word
	pop	%di			/ restore registers
	pop	%cx
	pop	%bx
	pop	%bp
	lret

/
/ mapit
/	fills a segment descriptor with stuff from registers
/	and points ES:DI to it
/
/ expects:
/	bx = limit
/	cx = lowbase
/	dl = hibase
/	dh = access byte
/
mapit:
	push	%ax
	mov	$<s>gdtptr,%ax			/ get seg of pntr to gdt
	mov	%ax,%ds				/ put it where I can use it
	les	gdtptr,%di			/ point to base of gdt
	mov	%bx,%es:[SCRATCH_SEL<<3](%di)	/ move limit into seg desc
	mov	%cx,%es:[[SCRATCH_SEL<<3]+2](%di) / move low base into seg desc
	movb	%dl,%es:[[SCRATCH_SEL<<3]+4](%di) / move hi base into seg desc
	movb	%dh,%es:[[SCRATCH_SEL<<3]+5](%di) / mv access byte into seg desc
	mov	$<s>scratchptr,%ax		/ get seg of pntr to scratchdesc
	mov	%ax,%ds				/ put it where I can use it
	les	scratchptr,%di			/ point to newly made desc
	pop	%ax
	ret

/
/ useracc
/	check user access
/
/	useracc( pointer, count, rw )
/
/	rw = 1 if want to test read access
/	rw = 0 if want to test write access
/
/	return 1 if access ok
/	return zero on no access
/
	.globl	useracc
useracc:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
/
/	first, check to see if selector is
/	valid
/
	mov	8(%bp),%bx		/ get selector
	mov	$USER_RPL,%ax		/ user privilege level
	and	%bx,%ax			/ requested priv. level illegal?
	cmp	$USER_RPL,%ax
	jne	error			/ yes, error and return
	lar	%bx,%dx			/ get the access rights byte (dh)
	jnz	error			/ can't get it, get out
	test	12(%bp)			/ no, read or write?
	jnz	chkread			/ read
chkwrite:
	verw	%bx			/ selector present and writable?
	jnz	error			/ no, error and return
	jmp	goon			/ yes, continue
chkread:
	verr	%bx			/ selector present and readable?
	jnz	error			/ no, error and return
goon:
	test	$LDT_TI,%bx		/ user trying to get to GDT?
	jz	error			/ yep, don't let'em get away with it
/
/	now (ugh), we have to check to see if the
/	offset is valid. if the segment that we want is
/	expand down, the given start offset must be > limit.
/	if the segment is expand up, the calculated end
/	offset must be <= limit
/
	mov	6(%bp),%si		/ get offset
	mov	10(%bp),%cx		/ get count
	dec	%cx			/ adj. count for end offset calculation
	add	%si,%cx			/ save final addr and check for overflow
	jc	error			/ we had overflow, so get out
	lsl	%bx,%ax			/ get limit
	jnz	error			/ can't get it - get out
/
/	Special case for full segment:
/	if the limit is FFFF, then all addresses in that segment
/	are legal, so don't do any more checking
/
	cmp	$0xFFFF,%ax		/ special case?
	je	great			/ yeah, everythings fine
	andb	$SD_EXPND_DN,%dh	/ expand down segment?
	jnz	expnd_dn		/ yes, handle limit differently
expnd_up:
	sub	%ax,%cx			/ final address > limit?
	ja	error			/ yes, error and exit
great:
	mov	$1,%ax			/ good return value
	pop	%bp
	lret
expnd_dn:
	sub	%ax,%si			/ offset <= limit?
	ja	great			/ no, user can access this
error:
	xor	%ax,%ax			/ bad return value
	pop	%bp
	lret
