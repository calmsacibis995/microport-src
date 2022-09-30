	.file	"bboot.s"

/
/       @(#)bboot.s	1.3 - 85/09/02
/

/ ********************************************************************
/
/
/	B O O T 
/
/	This is the boot strap loader for the UNIX-286 system
/
/ ********************************************************************

BOOT_SIZE	=	0xd00		/ Size of track to read
SZBSTACK	=	0x100		/ size of boot-stack
VLAB_START	=	0x180
VLAB_FLOFF	=	10
SZRMLAB		=	0x80
BOOT_BREAK	=	0x180
BOOT_RESUME	=	VLAB_START + SZRMLAB

	.bss

unit:
	. = . + 2

fname:
	. = . + 128

bstack:
	. = . + SZBSTACK

/
/	data areas for the 215g controller
/

/	channel control block

ccb:
	.=.+16

/	control invocation block

cib:
	.=.+20

/	i/o parameter block

iopb:
	.=.+30

/	data buffer

db:
	.=.+8

	.text

/ *********************************************************************
/
/	bootstrap
/
/	main entry point
/
/ *********************************************************************

/	parameter space

sec_size	=	0
aunit		=	2
prom_driver	=	4
boot_file_name	=	12
 
/	local data space

block_count	=	-2
buffer		=	-4
lvflags		=	-6
boot_unit	=	-8

	.globl	bootstrap

bootstrap:

/
/	first bytes will contain
/
/	jmp	to start of boot code
/
/	a db (for the 215g controller)
/	specifying the boots initial data block
/	for the media containing the file system
/	followed by fsdelta the offset in sectors to the
/	start of the file system.
/	These values will be filled in by the ib (install boot)
/	routine when this bootstrap track(s) are laid down.
/
/	(The values below are for a floppy based system)
/

	jmp	realbootstrap

	.globl	boot_db		/ the C part needs to know these values
boot_db:
	.value	0x0028
	.value	0x0200
	.value	0x0009
	.value	0x0102

	.globl	fsdelta		/ the C part needs to know this value	
fsdelta:
	.value	0x0012

realbootstrap:

	mov	%cs,%ax		/ initialise ds = cs (small model)
	mov	%ax,%ds
	mov	%bp,%sp		/ a little strange ! but we're never
				/ going to return to what called us
	sub	$8,%sp		/ create space for locals

/
/	read in rest of this stage
/

	mov	$1,block_count(%bp)
	mov	sec_size(%bp),%ax
	mov	%ax,buffer(%bp)

read_in_loop:

	push	aunit(%bp)
	xor	%ax,%ax
	push	%ax
	push	block_count(%bp)
	push	%cs
	push	buffer(%bp)
	lcall	*prom_driver(%bp)	/ PL/M style function call
	inc	block_count(%bp)
	mov	sec_size(%bp),%ax
	add	%ax,buffer(%bp)
	cmp	$BOOT_SIZE,buffer(%bp)
	jb	read_in_loop

/
/	save label info
/

	mov	[VLAB_START+VLAB_FLOFF],%ax
	mov	%ax,lvflags(%bp)

/
/	squash up code (over where label got loaded)
/

	mov	%ds,%ax
	mov	%ax,%es
	mov	$BOOT_RESUME,%si
	mov	$BOOT_BREAK,%di
	mov	$[[[BOOT_SIZE-BOOT_RESUME]+1]>>1],%cx
	cld
	rep
	smov

#if DEBUG
	int	$3			/ debug
#endif

/
/	initialise our driver
/

	cli
	mov	aunit(%bp),%ax
	and	$0x0007,%ax

	test	$0x10,lvflags(%bp)
	jnz	set_boot_unit
	or	$0x0310,%ax
	jmp	set_boot_unit

set_boot_unit:

	mov	%ax,boot_unit(%bp)

/
/	set up the wake up block
/

	xor	%ax,%ax
	mov	%ax,%es
	mov	$0x1000,%si
	mov	$0x0001,%es:(%si)
	mov	$ccb,%es:2(%si)
	mov	%ds,%ax
	mov	%ax,%es:4(%si)

/
/	set up ccb
/

	mov	$ccb,%si
	mov	$0xff01,(%si)	/ and set busy flag 

/
/	start the initialisation
/

	mov	$0x100,%dx
	movb	$2,%al
	outb	(%dx)
	mov	$15000,%cx     / first 15ms delay
delay1:
	loop	delay1
	movb	$0,%al
	outb	(%dx)
	mov	$15000,%cx	/second 15ms delay
delay2:
	loop	delay2
	movb	$1,%al
	outb	(%dx)

	mov	$0,%bx			/	used a lot
	mov	$cib+4,2(%si)
	mov	%ds,%ax
	mov	%ax,4(%si)
	mov	%bx,6(%si)
	mov	$0x0001,8(%si)
	mov	$ccb+14,10(%si)
	mov	%ds,%ax
	mov	%ax,12(%si)
	mov	$0x0004,14(%si)

/
/	set up the cib
/

	mov	$cib,%si
	mov	%bx,(%si)
	mov	%bx,2(%si)
	mov	%bx,4(%si)
	mov	%bx,6(%si)
	mov	$iopb,8(%si)
	mov	%ds,%ax
	mov	%ax,10(%si)
	mov	%bx,12(%si)
	mov	%bx,14(%si)

/
/	set up the iopb
/

	mov	$iopb,%si
	mov	%bx,(%si)
	mov	%bx,2(%si)
	mov	%bx,4(%si)
	mov	%bx,6(%si)
	mov	%bx,8(%si)	/ device code
	mov	%bx,10(%si)	/ function & unit
	mov	$1,12(%si)	/ modifier
	mov	%bx,14(%si)	/ cylinder
	mov	%bx,16(%si)	/ sector & head
	mov	%ds,%ax
	mov	%ax,20(%si)
	mov	%bx,22(%si)
	mov	%bx,24(%si)
	mov	%bx,26(%si)
	mov	%bx,28(%si)

/
/	clear db
/

	mov	$db,%si
	mov	%bx,(%si)
	mov	%bx,2(%si)
	mov	%bx,4(%si)
	mov	%bx,6(%si)

/
/	initialise all hard disc and floppy drives
/
/	for( ldev_code=0; ldev_code < 4 ; ldev_code += 3 )
/		for(lunit=0;lunit<4;lunit++)
/		{
/			if( this_is_the_one_we_booted_off)
/				init_db_with_our_values() ;
/			else
/				clear_db() ;
/			init_unit( lunit | ldev_code ) ;
/		}

/	%al == lunit
/	%ah == ldev_code

	mov	$0,%ax

loop_lunit:

	mov	$db,18+iopb		/ with zero-filled db

	cmp	%ax,boot_unit(%bp)
	jne	do_the_init
	mov	$boot_db,18+iopb	/ with our parameters

do_the_init:

	movb	%al,iopb+10		/ set unit #
	movb	%ah,iopb+8		/ set device code

	push	%ax
	call	startcontroller
	pop	%ax

	incb	%al			/ increment unit #
	testb	$0x4,%al
	jz	loop_lunit

	cmpb	$3,%ah
	je	done_init

	mov	$0x0310,%ax		/ go on to floppy units
	jmp	loop_lunit

/
/	save device code and unit # of the unit we're booting off
/

done_init:

	mov	boot_unit(%bp),%ax
	movb	%al,iopb+10		/ set unit #
	movb	%ah,iopb+8		/ set device code

/
/	save a copy of the file name and terminate it with NULL
/

	lds	boot_file_name(%bp),%si
	mov	%cs,%ax
	mov	%ax,%es
	mov	$fname,%di

getname:

	slodb
	cmpb	$0xd		/ NULL/CR/NL or anything else low terminates
	jle	gotname
	sstob
	jmp	getname

gotname:

	xorb	%al,%al
	sstob

/
/	call the bootstrap program
/

	mov	%cs,%bx
	mov	%bx,%ds
	mov	%bx,%ss
	mov	$bstack+SZBSTACK,%sp
	push	$fname
	call	bload			/ bload(fname);


/ *********************************************************************
/
/	iomove
/
/ *********************************************************************

	.globl	iomove

src_off	=	10
dst_off	=	12
dst_seg	=	14
count	=	16

iomove:
	push	%cx
	push	%bx
	push	%di
	push	%bp
	mov	%sp,%bp
	mov	dst_seg(%bp),%es
	mov	dst_off(%bp),%di
	mov	src_off(%bp),%si
	mov	count(%bp),%cx
	jcxz	iomdone			/ 0 ==> done
	rep				/ mult-times...
	smovb				/ move it.
iomdone:				/ return C-style
	jmp	cret

/ *********************************************************************
/
/	ljump
/
/ *********************************************************************

	.globl	ljump

ljump:
	pop	%ax
	lret

/ *********************************************************************
/
/	getDS
/
/ *********************************************************************

	.globl	getDS			/

getDS:
	mov	%ds,%ax
	ret				/ that's it.

/ *********************************************************************
/
/	halt
/
/ *********************************************************************

	.globl	halt
halt:
	sti
	hlt
	jmp	halt


/ *********************************************************************
/
/	getlong
/
/ *********************************************************************

	.globl	getlong

getlong:
	push	%bp
	mov	%sp,%bp
	les	4(%bp),%si
	mov	%es: (%si),%ax
	mov	%es: 2(%si),%dx
	leave
	ret

/ *********************************************************************
/
/	our driver for reading reading the file system
/
/ *********************************************************************

	.globl	driver

buffseg	=	16
buffoff	=	14
cylind	=	12
sechead	=	10

driver:
	push	%cx
	push	%bx
	push	%di
	push	%bp
	mov	%sp,%bp

/
/	set up pointer to buffer in iopb
/

	mov	$iopb,%si
	mov	buffoff(%bp),%ax
	mov	%ax,18(%si)
	mov	buffseg(%bp),%ax
	mov	%ax,20(%si)

	mov	cylind(%bp),%ax
	mov	%ax,iopb+14	/ cylinder
	mov	sechead(%bp),%ax
	movb	%al,iopb+17	/ sector
	movb	%ah,iopb+16	/ head

/
/	set up requested transfer count ( 512 bytes )
/

	mov	$0x200,iopb+22

/
/	do the read
/

	movb	$4,iopb+11

/
/	go for it
/

	call	startcontroller

	jmp	cret


/ *********************************************************************
/
/	startcontroller
/
/ *********************************************************************

startcontroller:

	enter	$0,$0

	movb	$1,%al
	mov	$0x100,%dx

waiting:
	testb	ccb+1
	jnz	waiting		/ wait for response
	movb	$0,cib+3	/ ?????

	outb	(%dx)

#if DEBUG1
	mov	$-1,%ax
waithere:
	dec	%ax
	test	%ax
	jnz	waitheredebug
	int $3
waitheredebug:
	testb	cib+3
	jz	waithere

#else
waithere:

	testb	cib+3
	jz	waithere
#endif
	movb	$0,cib+3
	leave
	ret

#if DEBUG
/ *********************************************************************
/
/	monitor
/
/ *********************************************************************

	.globl	monitor

monitor:

 	int	$3
	ret
#endif




/	C long div and mod (/ and %)
/       This must be kept in sync with /usr/src/lib/liba/ldivmod.s.
/
/	lmod called for long a,b,c
/		a = b % c
/	b in ax/dx
/	c in bx/cx
/	return result in ax/dx
/

#define	SAVEBHW		-2(%bp)
#define	SAVEBLW		-4(%bp)
#define	SAVECHW		-6(%bp)
#define	SAVECLW		-8(%bp)

#define LREMFLAG	$4
#define	DVNDFLAG	$2
#define	DVSRFLAG	$1
#define	BOTHFLAG	$3

	.globl	_lmod
_lmod:
	enter	$8,$0
	mov	LREMFLAG,%di		/ set lrem flag, clear sign flag
	jmp	chkdvnd

	.globl	_ulmod
_ulmod:
	enter	$8,$0
	mov	LREMFLAG,%di		/ set lrem flag, clear sign flag
	and	%cx,%cx			/ set condition flags for main code
	jmp	dvsrok

/
/	uldiv called for unsigned long a,b,c
/	(NOTE: only one of b or c being unsigned
/		causes the expression to be calculated as unsigned
/		as per standard C semantics)
/	ldiv called for long a,b,c
/		a = b / c
/	b in ax/dx
/	c in bx/cx
/	return result in ax/dx
/
	.globl	_uldiv
_uldiv:
	enter	$8,$0
	xor	%di,%di		/ merely clear the sign flags
	and	%cx,%cx		/ set condition flags for main code
	jmp	dvsrok		/ since the main part of ldiv
				/ is an unsigned algorithm

	.globl	_ldiv
_ldiv:
	enter	$8,$0

	xor	%di,%di		/clear sign flags
chkdvnd:
	and	%dx,%dx		/ is dividend neg?
	jns	dvndok
	xor	DVNDFLAG,%di	/ set neg dividend flag
	neg	%dx		/ and change sign
	neg	%ax		/ of dividend
	sbb	$0,%dx

dvndok:
/ dividend now unsigned
/	NOTE:	if dividend was 0x80000000, it still is,
/		but that is the correct unsigned value.
	and	%cx,%cx		/ is divisor neg?
	jns	dvsrok
	xor	DVSRFLAG,%di	/ set neg divisor flag
	neg	%cx		/ and change sign
	neg	%bx		/ of divisor
	sbb	$0,%cx

dvsrok:
/ divisor now unsigned (see NOTE for dividend)
/
/ Now determine how hard it is to divide.
/ Ultimately, the divisor must be reduced to one word in size,
/ so a divisor whose high word is already zero is much easier
/ to handle.
/
/ Condition codes for divisor high word are already set,
/ either by the 'and' for checking for negativity
/ or by the 'sbb' that fixes up the high word when needed.
/ (the jumps don't change the flags)
/

	jnz	fulldiv		/ jump if c[HW] != 0

/
/ c[HW] is zero.
/
/ This piece of code assumes that the case where
/ c[LW] > b[HW] is a high use path.
/ If it is determined that it is not a high use path
/ or if we get super stingy on code space, then the code
/ for this special case can be eliminated, since the
/ code for c[LW] <= b[HW] would get the right answer.
/
	cmp	%bx,%dx		/ this is %dx - %bx
	jnb	qtwodiv		/ use two word quotient code
				/ if c[LW] <= b[HW]
	div	%bx
	test	LREMFLAG,%di		/ is remainder desired?
	jnz	fixsrem			/ then go return it instead of quotient
	xor	%dx,%dx		/ one word quotient, so zero a[HW]
	jmp	fixsign

qtwodiv:
/ c[HW] == 0 && c[LW] <= b[HW]
/ This is the only case where the quotient has two significant words.
/
/ The division takes places as if we were dealing with numbers
/ base 256, where the dividend is a two-digit number and
/ the divisor is a one-digit number.
/
	mov	%ax,%si		/ squirrel away b[LW]
	mov	%dx,%ax		/ set up first divide
	xor	%dx,%dx
	div	%bx		/ producing a[HW] in %ax
				/ and partial remainder for
				/ second divide in %dx (the right place)
	xchg	%ax,%si		/ save a[HW] and set up second divide
	div	%bx		/ producing a[LW] in %ax
	test	LREMFLAG,%di		/ is remainder desired?
	jnz	fixsrem			/ then go return it instead of quotient
	mov	%si,%dx		/ a now in ax/dx
	jmp	fixsign

fixsrem:
	mov	%dx,%ax
	xor	%dx,%dx
	and	DVNDFLAG,%di
	jmp	fixsign

fulldiv:
/
/ C[HW] != 0
/ so things get a little trickier to think about.
/ Now we think of the numbers base 256 (2**8).
/ So, in that form, the divisor is either a three or four digit number.
/ If the divisor is a four digit number, the numerical error
/ of using only the high words of both b and c to determine
/ the quotient are such that you get either a or a+1.
/ If you use the notation b(N) to mean the digit for 256**N position,
/ then this corresponds to the greatest integer in the quotient:
/	b(3)b(2)/c(3)c(2)
/ which is always >= the real quotient.  To see that it is never
/ too high by more than 1, consider the following:
/ First:
/	b(3)b(2)/c(3)c(2) = b(3)b(2)00/c(3)c(2)00
/ Then the error is:
/	b(3)b(2)00/c(3)c(2)00 - b(3)b(2)b(1)b(0)/c(3)c(2)c(1)c(0)
/ which can be simplified to:
/	b(3)b(2)00/c(3)c(2)c(1)c(0) * c(1)c(0)/c(3)c(2)00
/		- b(1)b(0)/c(3)c(2)c(1)c(0)
/ If we use M to the max digit for the radix in use, the largest
/ value of the positive (first) term is:
/	MM00/10MM * MM/1000 < .MM *.MM < 1
/ Since the difference under consideration was using the rational
/ values of the quotients rather than the greatest integer in them,
/ the greatest integer in b[HW]/c[HW] can be off by 1 on the high side
/ (but no more than 1).
/ A similar analysis is behind other situations covered below.
/ Those situations will only quote the results.
/
/ The full divide algorithm needs the original b and c values
/ and all of the registers, so the values of b and c must be saved.
/
	mov	%bx,SAVECLW
	mov	%cx,SAVECHW
	mov	%ax,SAVEBLW
	mov	%dx,SAVEBHW
/ Now check the high byte of c:
	andb	%ch,%ch
	jz	shftdiv		/ jump if its high byte is zero

/ c(3) != 0 and the above analysis applies, so do it.
	mov	%cx,%bx		/ set up c[HW]
	mov	%dx,%ax		/ and b[HW]
	xor	%dx,%dx		/ and clear high dividend bits
	jmp	dodiv

shftdiv:
/ c(3) == 0.
/ Shifting by bytes to zero the high word has to high of a
/ potential error: almost M.  More special cases could be
/ found, but we are getting to the point of diminishing returns.
/ for the general case that is left, ther radix must be 2
/ and shifts must be done minimally to get the potential error
/ down to 1.  Further, the only convenient shift below bytes
/ is single bits.  So the rest go through a bit shift loop.
/
	shr	%dx		/ low bit to carry
	rcr	%ax		/ carry to high bit
	shr	%cx		/ the same with the divisor
	rcr	%bx
	and	%cx,%cx		/ is c[HW] still nonzero?
	jnz	shftdiv

dodiv:
	div	%bx		/ %bx now the pseudo divisor
/ Now we must determine if we are off by 1.
	mov	%ax,%si		/ save pseudo-quotient (PQ)
	mov	SAVECHW,%ax	/ calc. c * PQ
				/ = c[LW] * PQ + 2**16 * c[HW] * PQ
	mul	%si		/ where * is unsigned.
	mov	%ax,%cx		/ save c[HW] * PQ
	mov	SAVECLW,%ax
	mul	%si		/ c[LW] * PQ
	add	%cx,%dx		/ finishes c * PQ
/ if c * PQ > b, we are off by 1:
	cmp	%dx,SAVEBHW	/ this is SAVEBHW - %dx
	jb	offby1
	ja	setqval
	cmp	%ax,SAVEBLW	/ this is SAVEBLW - %ax
	jae	setqval

offby1:
	test	LREMFLAG,%di	/ is remainder desired?
	jnz	roffby1		/ then go fix up product to get remainder
	dec	%si

setqval:
	test	LREMFLAG,%di	/ is remainder desired?
	jnz	setrval		/ then go calculate it
	mov	%si,%ax
	xor	%dx,%dx

/ all that is left is establishing the sign of the quotient

fixsign:
	and	%di,%di
	je	divret
	xor	BOTHFLAG,%di
	je	divret
	neg	%dx
	neg	%ax
	sbb	$0,%dx

divret:
	leave

#if LARGE_M | HUGE_M | COMPACT_M
	lret
#else
	ret
#endif

roffby1:			/ PQ was 1 too big, so fix product
	sub	SAVECLW,%ax
	sbb	SAVECHW,%dx
setrval:
	mov	%ax,%bx
	mov	%dx,%cx
	mov	SAVEBLW,%ax
	mov	SAVEBHW,%dx
	sub	%bx,%ax
	sbb	%cx,%dx
	and	DVNDFLAG,%di
	jmp	fixsign

/ C long multiply
/ This must be kept in sync with /usr/src/lib/liba/lmul.s.

/
/	called for long a,b,c
/		a  = b * c
/
/	b in bx/cx
/	c in ax/dx
/	result returned in ax/dx
/
/	can trash all non-segment registers
/
/	algorithm:
/		If you think of the two numbers being multiplied
/		as unsigned two-digit numbers base 2**16, then
/		(assuming unsigned * is used)
/		a trivial algorithm for the lowest two digits
/		of their product is:
/			a = b(LW) * c(LW)
/			    + 2**16 * b(LW) * c(HW)
/			    + 2**16 * b(HW) * c(LW)
/

#define	RHS_low		-2(%bp)

	.text
	.globl	_lmul

_lmul:
	enter	$2,$0

	mov	%ax,%si		/ save b(LW) temporarily in %si
	mov	%dx,%di		/ save b(HW) in %di
	mul	%bx		/ b(LW) * c(LW)
	mov	%ax,RHS_low	/ finished with a(LW), so squirrel it away
	mov	%si,%ax		/ last use of b(LW), so %si now avail.
	mov	%dx,%si		/ %si now accumulating a(HW)
	mul	%cx		/ b(LW) * c(HW)
	add	%ax,%si		/ ignore any overflow in %dx
	mov	%di,%ax		/ b(HW)
	mul	%bx		/ b(HW) * c(LW)
	add	%ax,%si		/ a(HW) now finished
	mov	RHS_low,%ax	/ return a(LW)
	mov	%si,%dx		/ return a(HW)

	leave
#if LARGE_M | HUGE_M | COMPACT_M
	lret
#else
	ret
#endif



/ *********************************************************************
/
/	cret
/
/ *********************************************************************

cret:
	leave
	pop	%di
	pop	%bx
	pop	%cx
	ret

/ *********************************************************************
/
/	setgdt
/
/	|-------------------------------------------------------|
/	|   type		|	phys addr (h.b.)	|
/	|-------------------------------------------------------| 12
/	|   physical address low 2 bytes			|
/	|-------------------------------------------------------| 10
/	|		segment size				|
/	|-------------------------------------------------------|  8
/	|	GDT entry address (real mode ) segment		|
/	|-------------------------------------------------------|  6
/	|	GDT entry address (real mode) offset		|
/	|-------------------------------------------------------|  4
/	|		return ip   				|
/	|-------------------------------------------------------|
/	|		old bp      				| 
/	|-------------------------------------------------------| <-- bp
/ *********************************************************************

	.globl	setgdt

setgdt:
	push	%bp
	mov	%sp,%bp
	les	4(%bp),%di
	mov	8(%bp),%ax
	ssto
	mov	10(%bp),%ax
	ssto
	mov	12(%bp),%ax
	ssto
	leave
	ret

/ *********************************************************************
/
/	zeroise a segment
/
/
/	|-------------------------------------------------------| 12
/	|   		size to zeroise (high 2 bytes)		|
/	|-------------------------------------------------------| 10
/	|		size to zeroise (low  2 bytes)		|
/	|-------------------------------------------------------|  8
/	|	Zeroise   address (real mode ) segment		|
/	|-------------------------------------------------------|  6
/	|		0					|
/	|-------------------------------------------------------|  4
/	|		return ip   				|
/	|-------------------------------------------------------|
/	|		old bp      				| 
/	|-------------------------------------------------------| <-- bp
/
/
/	ASSUMPTIONS	-	WARNING
/
/	1. Start address is on a 16-byte boundary
/	2. Amount of initialisation is even number of bytes
/
/ *********************************************************************

	.globl	zeroise

zeroise:
	push	%bp
	mov	%sp,%bp

/
/	deal with segment worths first
/

	les	4(%bp),%di		/ load es , di pair
	xor	%ax,%ax		/ zeroise %ax

	mov	10(%bp),%cx		/ number of segments
	jcxz	z_rest			/ none

z_seg:
	push	%cx
	mov	$0x8000,%cx
	rep
	ssto

	mov	%es,%cx		/ increment seg. address by seg.
	add	$0x1000,%cx
	mov	%cx,%es
	mov	$0,%di		/ never trust a micro

	pop	%cx
	loop	z_seg

/
/	now with any remaining bytes
/

z_rest:
	mov	8(%bp),%cx
	shr	%cx		/ bytes to words
	rep
	ssto

	leave
	ret


#if RELOC
/ *********************************************************************
/
/	reloc a large amount of text
/
/
/	|-------------------------------------------------------| 16
/	|   		size to reloc (high 2 bytes)		|
/	|-------------------------------------------------------| 14
/	|		size to reloc (low  2 bytes)		|
/	|-------------------------------------------------------| 12
/	|	Destination address (offset)          		|
/	|-------------------------------------------------------| 10
/	|	Destination address (segment)			|
/	|-------------------------------------------------------|  8
/	|	Source address (offset )              		|
/	|-------------------------------------------------------|  6
/	|	Source address (segment)			|
/	|-------------------------------------------------------|  4
/	|		return ip   				|
/	|-------------------------------------------------------|  2
/	|		old bp      				| 
/	|-------------------------------------------------------| <-- bp
/
/
/	ASSUMPTIONS	-	WARNING
/
/	1. Start address is on a 16-byte boundary
/	2. Amount of relocation is even number of bytes
/
/ *********************************************************************

	.globl	reloc

reloc:
	push	%bp
	mov	%sp,%bp
	push	%ds


/	deal with segment worths first


	les	8(%bp),%di		/ load es , di pair
	lds	4(%bp),%si

	mov	14(%bp),%cx		/ number of segments
	jcxz	r_rest			/ none

r_seg:
	push	%cx
	mov	$0x8000,%cx
	rep
	smov

	mov	%es,%cx		/ increment destination segment
	add	$0x1000,%cx
	mov	%cx,%es

	mov	%ds,%cx		/ increment source segment
	add	$0x1000,%cx
	mov	%cx,%ds

	mov	$0,%di		/ never trust a micro

	pop	%cx
	loop	r_seg


/	now with any remaining bytes


r_rest:
	mov	12(%bp),%cx
	shr	%cx		/ bytes to words
	rep
	smov

	pop	%ds
	leave
	ret
#endif
