	.file	"misc.s"
/ uportid = "@(#)misc.s	Microport Rev Id  1.3.8 10/19/86";
/
/ @(#)misc.s	1.21
/
/ Modification History:		uport!dwight Sun Oct 19 14:00:22 PDT 1986
/ The General upgrade to the IBM AT involves the following:
/	1) Lance Norskog's splbio() define
/	2) Making curlevel global.
/ All of these are ifdef'd on IBMAT
/ M002: uport!mike Tue Oct 14 03:04:02 PDT 1986
/	interval count to variable.
/ M003: uport!doug Wed Dec 17 08:50:33 PDT 1986
/	always use spl tables (patchability)
/ M004: uport!rex	Tue Aug 11 12:20:01 PDT 1987
/	modifications to the PIC handling have been tested here and in
/	trap.s.  All of the changes are ifdef'ed with PICFIX[1-5] which mean:
/
/	PICFIX1 turns on all the Locus FIX's dealing with the 8259.
/	PICFIX2 turns on Special Fully Nested Mode on the Master PIC
/		and modifies the EOI code to accomodate the new mode.
/	PICFIX3 prevents an automatic SPL for clock interrupts.
/	PICFIX4 modifies spl#() calls to use the patchable spl tables.
/		These calls were using 8259.h constants.
/	PICFIX5 adds code to patch the spl0 mask  to set the bit corresponding
/		to any interrupt handler that is currently in service.
/
/	See iAPX286/Makefile or iAPX286/MAKE  for the current
/	PICFIX definitions.

#include	"sys/8259.h"
#include	"sys/clock.h"

	.text

/
/ clkstart
/	initialize and start the clock
/
/ clockon
/	just turn on the clock
/
/ clockoff
/	turn off the clock
/
 	.data
 	.globl	clocktic	/ M002
clocktic:	.value	RTCCNT	/ number of tics per 60 HZ M002

	.text
	.globl	clkstart
	.globl	clockon
	.globl	clockoff

#ifdef	ATMERGE
/ Merge uses the realtime clock for UNIX

	.globl	dclkinit
	.globl	dclkintr
	.globl	dos_clockon
	.globl	dos_clockoff

clkstart:
clockon:

	movb	$0x0a, %al
	outb	0x70
	jmp	.+2
	movb	$RTC_INITA, %al
	outb	0x71			/Write RTC status reg A
	jmp	.+2

	movb	$0x0b, %al
	outb	0x70
	jmp	.+2
	movb	$RTC_INITB, %al
	outb	0x71			/Write RTC status reg B

	lcall	spl0			/ enable interrupts from PIC
	sti				/ INTERRUPTS ON
	lret


clockoff:

	movb	$0x0b, %al
	outb	0x70
	jmp	.+2
	movb	$RTC_OFFB, %al
	outb	0x71			/Write RTC status reg B

	lret

dclkinit:

	lcall	dos_clockoff

	movb	$RTCMODE,%al		/ get mode word
	mov	$CTCMODE,%dx
	outb	(%dx)			/ kick it
	mov	$CNT0MODE,%dx		/ CTC counter 0 port
	mov	$DOSCNT,%ax	       	/ interval count for dos
	outb	(%dx)			/ output lsb
	movb	%ah,%al			/ output ...
	outb	(%dx)			/ ... msb

	lret

dclkintr:
	push	%ds
	push	%ax
	pushf
/ First EOI the timer interrupt
	cli
	movb	$0x60, %al
	outb	0x20
/ Increment dosticks if DOS is around
	mov	$<s>usw_data, %ax
	mov	%ax, %ds
	cmp	$0, dosok	/ is dos there?
	je	notok
	inc	usw_dosticks	/ yes, but since he doesn't have the screen
				/ we can't give him the interrupt.
notok:
READ_ISR  =  0x0b		/ 8259 command to read in-service register
/ Now read the current isr regs into usw_isr
	movb	$READ_ISR, %al
	outb	PICSSTAT
	jmp	.+2
	inb	PICSSTAT
	movb	%al, %ah
	movb	$READ_ISR, %al
	outb	PICMSTAT
	jmp	.+2
	inb	PICMSTAT
	mov	%ax, usw_isr
	lcall	<s>popff, popff
	pop	%ax
	pop	%ds
	lret

dos_clockoff:
	push	$0x0001			/ disable clock
	lcall	msmdisable
	add	$2,%sp
	lret

dos_clockon:
	push	$0x0001			/ enable clock
	lcall	msmenable
	add	$2,%sp
	lret

#else	/* -ATMERGE */
clkstart:
clockon:
	movb	$RTCMODE,%al		/ get mode word
	mov	$CTCMODE,%dx
	outb	(%dx)			/ kick it
	mov	$CNT0MODE,%dx		/ CTC counter 0 port
	mov	$RTCCNT,%ax		/ interval count
	mov	$<s>clocktic,%ax	/ load segment ...      M002
	mov	%ax,%ds			/ ... of interval count M002
	mov	clocktic,%ax		/ interval count        M002
	outb	(%dx)			/ output lsb
	movb	%ah,%al			/ output ...
	outb	(%dx)			/ ... msb
	lcall	spl0			/ enable interrupts from PIC
	sti				/ INTERRUPTS ON
	lret

clockoff:
	movb	$RTCMODE,%al		/ get mode word
	mov	$CTCMODE,%dx		/ CTC counter 0 port
	outb	(%dx)			/ kick it - clock will hang
	lret
#endif /* ATMERGE */

/
/ spl?()
/	send the PIC the appropriate masks to simulate
/	priority level
/
/	returns previous level
/
/	spl0()	enables all interrupts ( clock, 544 sio, 215 disk, console )
/	spl4()	disables console
/	spl5()	disables console and 544 sio controller interrupts
/	spl6()	disables cons, 544, and 215 disk interrupts
/	spl7()	disables everything
/	splhi()	disables everything
/
/	splx( oldlevel )	restores interrupt to previous state
/
	.data
#ifdef IBMAT
	.globl	curlevel
#endif IBMAT
curlevel:	.value	0	/ current spl level

	.text

#ifdef	PICFIX1
	.globl	spl0
spl0:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	push	curlevel		/ for return of old level
	mov	$0,curlevel		/ set current level
	movb	$SPL0MASTER, %al	/ get master PIC mask for this level
	movb	$SPL0SLAVE, %ah		/ get slave PIC mask for this level
	jmp	splout

	.globl	spl4
#ifdef IBMAT
	.globl	splbio
splbio:
#endif IBMAT
spl4:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	push	curlevel		/ for return of old level
	mov	$4, curlevel		/ set current level
	movb	$SPL4MASTER, %al	/ get master PIC mask for this level
	movb	$SPL4SLAVE, %ah		/ get slave PIC mask for this level
	jmp	splout

	.globl	spl5
spl5:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	push	curlevel		/ for return of old level
	mov	$5,curlevel		/ set current level
	movb	$SPL5MASTER, %al	/ get master PIC mask for this level
	movb	$SPL5SLAVE, %ah		/ get slave PIC mask for this level
	jmp	splout

	.globl	spl6
spl6:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	push	curlevel		/ for return of old level
	mov	$6,curlevel		/ set current level
	movb	$SPL6MASTER, %al	/ get master PIC mask for this level
	movb	$SPL6SLAVE, %ah		/ get slave PIC mask for this level
	jmp 	splout

	.globl	spl7
	.globl	splhi
spl7:
splhi:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	push	curlevel		/ for return of old level
	mov	$7,curlevel		/ set current level
	movb	$SPL7MASTER, %al	/ get master PIC mask for this level
	movb	$SPL7SLAVE, %ah		/ get slave PIC mask for this level
splout:
#ifdef	PICFIX5
	lcall	<s>setpicmsk,setpicmsk	/ do common code there
#else	/* ! PICFIX5 */
#ifdef ATMERGE 
	lcall	<s>mergemask, mergemask / adjust mask as merge needs it
#endif /* ATMERGE */
#ifdef	PICFIX4
/ spltbl:					/ spl level in bx
	mov	curlevel,%bx
	mov	$<s>mastbl,%ax		/ load segment ...
	mov	%ax,%es			/ ... of master PIC table
	mov	$mastbl,%si		/ load offset of master PIC table
	movb	%es:(%bx,%si),%al	/ get master mask for this PIC level
	mov	$PICMASTER,%dx		/ get address of PIC master port
	outb	(%dx)			/ set master PIC to this level
	mov	$<s>slavtbl,%ax		/ load segment ...
	mov	%ax,%es			/ ... of slave PIC table
	mov	$slavtbl,%si		/ load offset of slave PIC table
	movb	%es:(%bx,%si),%al	/ get slave mask for this PIC level
	mov	$PICSLAVE,%dx		/ get address of slave PIC port
	outb	(%dx)			/ set slave pic to this level
/	ret
#else	/* ! PICFIX4 */
	mov	$PICMASTER, %dx		/ get address of master PIC
	outb	(%dx)			/ kick master
	movb	%ah, %al		/ slave PIC mask
	mov	$PICSLAVE, %dx		/ get address of slave PIC
	outb	(%dx)			/ kick slave
#endif	/* ! PICFIX4 */
#endif	/* ! PICFIX5 */
	pop	%ax			/ return value (old level)
	lcall	<s>popff, popff		/ don't trust 286 popf instruction
	lret

#ifdef	PICFIX5
/ channel number 0-f is passed in the bx register
	.globl	disablepicint
disablepicint:
	mov	$1,%ax
	mov	%bx,%cx
	sub	$8,%cx
	ja	slavebit
	mov	%bx,%cx
	shl	%cl,%ax
	mov	$<s>mastbl,%dx		/ load segment ...
	mov	%dx,%es			/ ... of master PIC table
	mov	$mastbl,%si		/ load offset of master PIC table
	movb	%es:0(%si),%dl	/ or with master mask for spl0()
	jmp 	disreturn
slavebit:
	shl	%cl,%ax
	mov	$<s>slavtbl, %dx	/ load segment ...
	mov	%dx, %es		/ ... of slave PIC table
	mov	$slavtbl,%si		/ load offset of slave PIC table
	movb	%es:0(%si),%dl	/ or with slave mask for spl0()
disreturn:
	orb	%al,%dl
	movb	%dl,%es:0(%si)	/ save as new spl0 mask
	lret

	.globl	enablepicint
enablepicint:
	mov	$1,%ax
	mov	%bx,%cx
	sub	$8,%cx
	ja	enslavebit
	mov	%bx,%cx
	shl	%cl,%ax
	mov	$<s>mastbl,%dx		/ load segment ...
	mov	%dx,%es			/ ... of master PIC table
	mov	$mastbl,%si		/ load offset of master PIC table
	movb	%es:0(%si),%dl	/ and with master mask for spl0()
	jmp 	enreturn
enslavebit:
	shl	%cl,%ax
	mov	$<s>slavtbl, %dx	/ load segment ...
	mov	%dx, %es		/ ... of slave PIC table
	mov	$slavtbl,%si		/ load offset of slave PIC table
	movb	%es:0(%si),%dl	/ and with slave mask for spl0()
enreturn:
	not	%ax
	andb	%al,%dl
	movb	%dl,%es:0(%si)	/ save as new spl0 mask
	lret


setpicmsk:
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%bx
	mov	$<s>mastbl,%ax		/ load segment ...
	mov	%ax,%es			/ ... of master PIC table
	mov	$mastbl,%si		/ load offset of master PIC table
	movb	%es:(%bx,%si),%al	/ get master mask for this PIC level
	movb	%es:0(%si),%dl		/ get master mask for spl0()
	orb	%dl,%al		/ or with master mask for spl0()
	mov	$PICMASTER,%dx		/ get address of PIC master port
	outb	(%dx)			/ set master PIC to this level
	mov	$<s>slavtbl, %dx	/ load segment ...
	mov	%dx, %es		/ ... of slave PIC table
	mov	$slavtbl,%si		/ load offset of slave PIC table
	movb	%es:(%bx,%si),%ah	/ get slave mask for this PIC level
	movb	%es:0(%si),%dl	/ get slave mask for this slp0() level
	orb	%dl,%ah		/ or with mask for spl0()
#ifdef ATMERGE
	lcall	<s>mergemask, mergemask / adjust mask as merge needs it
#endif /* ATMERGE */
	movb	%ah, %al
	mov	$PICSLAVE,%dx		/ get address of slave PIC port
	outb	(%dx)			/ set slave pic to this level
	lret
#endif	/* ! PICFIX5 */

	.globl	splx
splx:
	push	%bp			/ establish ...
	mov	%sp, %bp		/ ... stack frame
	push	%bx			/ save what I'm gonna use
	pushf				/ save flags
		/ Do the cli here where it will keep curlevel consistent
	cli				/ no interrupts while messing with PIC
	mov	6(%bp), %bx		/ get spl level to go to
	mov	$<s>curlevel, %dx
	mov	%dx, %ds
	push	curlevel
	mov	%bx, curlevel
#ifdef	PICFIX5
	lcall	<s>setpicmsk,setpicmsk	/ do common code there
#else	/* ! PICFIX5 */
	mov	$<s>mastbl, %dx		/ load segment ...
	mov	%dx, %es		/ ... of master PIC table
	mov	$mastbl, %si		/ load offset of master PIC table
	movb	%es:(%bx,%si), %al	/ get master mask for this PIC level
	mov	$<s>slavtbl, %dx	/ load segment ...
	mov	%dx, %es		/ ... of slave PIC table
	mov	$slavtbl, %si		/ load offset of slave PIC table
	movb	%es:(%bx,%si), %ah	/ get slave mask for this PIC level
#ifdef ATMERGE
	lcall	<s>mergemask, mergemask / adjust mask as merge needs it
#endif /* ATMERGE */
	mov	$PICMASTER, %dx		/ get address of PIC master port
	outb	(%dx)			/ set master PIC to this level
	movb	%ah, %al
	mov	$PICSLAVE, %dx		/ get address of slave PIC port
	outb	(%dx)			/ set slave pic to this level
#endif	/* ! PICFIX5 */
	pop	%ax			/ pop old curlevel
	lcall	<s>popff, popff		/ don't trust 286 popf instruction
	pop	%bx			/ restore other regs
	pop	%bp
	lret



/
/  eoi
/ 	send a specific End-Of-Interrupt command to the
/ 	master PIC if the interrupt came from the master; 
/ 	otherwise, send an End-Of-Interrupt to the slave, then
/ 	to the master.
/ 
	.globl	aeoi			/ M000

aeoi:	
	push	%bp
	mov	%sp,%bp		/ get the base of parameters
	pushf
	push	%ds
	push	%ax
	mov	6(%bp),%ax	
	cmp	$0x0f, %ax / is vector out of range of the slave?
	ja	eoi_ret
	cli
	cmp	$8, %ax
	jb	eoi_master
eoi_slave:
	subb	$8, %al
	orb	$0x60, %al
	outb	0xa0			/ eoi the slave
#ifdef	PICFIX2
	jmp	.+2
	movb	$0x0b, %al		/ select ISR read
	outb	0xa0
	jmp	.+2
	inb	0xa0			/ read ISR
	cmpb	$0x00,%al		/ more ISR bits set?
	jne	eoi_ret			/ yes
#endif	/* PICFIX2 */
	movb	$0x62, %al		/ Specific eoi for slave
	outb	0x20			/  to the master
	jmp	eoi_ret
eoi_master:
	orb	$0x60, %al
	outb	0x20
	/jmp	eoi_ret
eoi_ret:
#ifdef ATMERGE
	mov	$<s>usw_isr, %ax	/ read in new ISR registers...
	mov	%ax, %ds
	movb	$0x0b, %al
	outb	0x20
	jmp	.+2
	outb	0x0a0
	jmp	.+2
	inb	0x0a0
	jmp	.+2
	xchgb	%al, %ah
	inb	0x20
	mov	%ax, usw_isr		/ ...and update our copy
#endif /* ATMERGE */
	pop	%ax
	pop	%ds
	lcall	<s>popff, popff
	pop	%bp
	lret

popff:
#ifdef PICFIX
	popf
	lret
#else /* ! PICFIX */
	iret
#endif /* ! PICFIX */

#else /* ! PICFIX1 */
	.globl	spl0
spl0:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
					/ M003 ...
	mov	$0,%bx			/ set spl level to go to
	call	spltbl			/ set spl level from master & slave tbls
					/ ... end M003 changes
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%ax		/ return old level
	mov	$0,curlevel		/ set current level
	popf				/ restore cli/sti state
	lret

	.globl	spl4
#ifdef IBMAT
	.globl	splbio
splbio:	
#endif IBMAT
spl4:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
					/ M003 ...
	mov	$4,%bx			/ set spl level to go to
	call	spltbl			/ set spl level from master & slave tbls
					/ ... end M003 changes
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%ax		/ return old level
	mov	$4,curlevel		/ set current level
	popf				/ restore cli/sti state
	lret

	.globl	spl5
spl5:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
					/ M003 ...
	mov	$5,%bx			/ set spl level to go to
	call	spltbl			/ set spl level from master & slave tbls
					/ ... end M003 changes
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%ax		/ return old level
	mov	$5,curlevel		/ set current level
	popf				/ restore cli/sti state
	lret

	.globl	spl6
spl6:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
					/ M003 ...
	mov	$6,%bx			/ set spl level to go to
	call	spltbl			/ set spl level from master & slave tbls
					/ ... end M003 changes
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%ax		/ return old level
	mov	$6,curlevel		/ set current level
	popf				/ restore cli/sti state
	lret

	.globl	spl7
	.globl	splhi
spl7:
splhi:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
					/ M003 ...
	mov	$7,%bx			/ set spl level to go to
	call	spltbl			/ set spl level from master & slave tbls
					/ ... end M003 changes
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%ax		/ return old level
	mov	$7,curlevel		/ set current level
	popf				/ restore cli/sti state
	lret
					/ M003 ...
spltbl:					/ spl level in bx
	mov	$<s>mastbl,%ax		/ load segment ...
	mov	%ax,%es			/ ... of master PIC table
	mov	$mastbl,%si		/ load offset of master PIC table
	movb	%es:(%bx,%si),%al	/ get master mask for this PIC level
	mov	$PICMASTER,%dx		/ get address of PIC master port
	outb	(%dx)			/ set master PIC to this level
	mov	$<s>slavtbl,%ax		/ load segment ...
	mov	%ax,%es			/ ... of slave PIC table
	mov	$slavtbl,%si		/ load offset of slave PIC table
	movb	%es:(%bx,%si),%al	/ get slave mask for this PIC level
	mov	$PICSLAVE,%dx		/ get address of slave PIC port
	outb	(%dx)			/ set slave pic to this level
	ret
					/ ... end M003 changes

	.globl	splx
splx:
	push	%bp			/ establish ...
	mov	%sp,%bp			/ ... stack frame
	push	%bx			/ save what I'm gonna use
	pushf				/ save flags
	mov	6(%bp),%bx		/ get spl level to go to
	mov	$<s>mastbl,%ax		/ load segment ...
	mov	%ax,%es			/ ... of master PIC table
	mov	$mastbl,%si		/ load offset of master PIC table
	movb	%es:(%bx,%si),%al	/ get master mask for this PIC level
	mov	$PICMASTER,%dx		/ get address of PIC master port
	cli				/ no interrupts while messing with PIC
	outb	(%dx)			/ set master PIC to this level
	mov	$<s>slavtbl,%ax		/ load segment ...
	mov	%ax,%es			/ ... of slave PIC table
	mov	$slavtbl,%si		/ load offset of slave PIC table
	movb	%es:(%bx,%si),%al	/ get slave mask for this PIC level
	mov	$PICSLAVE,%dx		/ get address of slave PIC port
	outb	(%dx)			/ set slave pic to this level
	mov	$<s>curlevel,%dx	/ load segment ...
	mov	%dx,%ds			/ ... of curlevel
	mov	%bx,curlevel		/ set current level
	popf				/ restore cli/sti state
	pop	%bx			/ restore other regs
	pop	%bp
	lret
#endif /* ! PICFIX1 */ 

#ifdef ATMERGE
	.globl	mergemask	/ only global for debugging
/ ax is the mask that Unix wants to write to the 8259's.  Change it as
/ merge wants, so as not to enable any interrupts that must be off at
/ the moment.  ds is the seg of curlevel.
SPLDOS = 4
mergemask:
	push	%bx
	push	%cx
	push	%dx
	mov	$0xffff, %bx		/ DOS ints are all off at high spl
	mov	$<s>curlevel, %cx
	mov	%cx, %ds
	cmp	$SPLDOS, curlevel
	jae	above
	mov	$<s>dosintmask, %bx	/ we're below SPLDOS
	mov	%bx, %ds
	mov	dosintmask, %bx
above:					/ bx is mask of DOS spl bits
	mov	$<s>usw_devassign, %cx
	mov	%cx, %ds
	mov	usw_devassign, %cx	/ cx is mask of assigned UNIX devs
	mov	%cx, %dx
	not	%dx			/ dx is mask of assigned DOS devs
	and	%cx, %ax		/ (UNIX mask) & (UNIX spl bits)
	and	%dx, %bx		/ (DOS mask) & (DOS spl bits)
	or	%bx, %ax		/ combined
	mov	$<s>msmmask, %cx
	mov	%cx, %ds
	or	msmmask, %ax		/ and don't allow msmdisable'd ints
	pop	%dx
	pop	%cx
	pop	%bx
	lret

	.globl	msmenable
msmenable:
	push	%bp
	mov	%sp, %bp
	pushf
	mov	6(%bp), %cx
/ cl is mask to enable in master, ch in slave
	notb	%cl
	mov	$<s>msmmask, %dx
	mov	%dx, %ds
	andb	%cl, msmmask		/ so spl's will treat this one normally
	cli
	inb	PICMASTER		/ current master mask
	andb	%cl, %al
	outb	PICMASTER
	notb	%ch
	andb	%ch, msmmask+1		/ again, so spl's will treat this one
	inb	PICSLAVE		/ normally
	andb	%ch, %al
	outb	PICSLAVE
	lcall	<s>popff, popff
	pop	%bp
	lret

	.globl	msmdisable
msmdisable:
	push	%bp
	mov	%sp, %bp
	pushf
	mov	6(%bp),%cx
/ cl is mask to disable in master, ch in slave
	mov	$<s>msmmask, %dx
	mov	%dx, %ds
	orb	%cl, msmmask		/ so spl's will keep these masked
	cli
	inb	PICMASTER		/ current master mask
	orb	%cl, %al
	outb	PICMASTER
	orb	%ch, msmmask+1		/ again, so spl's will keep these masked
	inb	PICSLAVE
	orb	%ch, %al
	outb	PICSLAVE
	lcall	<s>popff, popff
	pop	%bp
	lret
#endif /* ATMERGE */

/
/ in, out, inb, outb
/	input and output routines to be called from C.
/
/	word in( port )
/		int port;
/
/	char inb( port )
/		int port;
/
/	out( port, word )
/		int port;
/		int word;
/
/	outb( port, byte )
/		int port;
/		char byte;
/
/	outb2( port, byte, byte2 )
/		int port;
/		char byte;		byte  -> (port)
/		char byte2;		byte2 -> (port)
/
/	outb21( port, byte, byte2 )
/		int port;
/		char byte;		byte  -> (port)
/		char byte2;		byte2 -> (port + 1)
/
/	outb23( port, reg, data )
/		int port;		reg           -> port
/		int reg;		hibyte (data) -> port + 1
/		int data;		reg + 1       -> port
/					lobyte (data) -> port + 1
/

	.globl	in
in:	push	%bp		/ save bp
	mov	%sp,%bp		/ get the base of parameters
	mov	6(%bp),%dx	/ get port number
	in	(%dx)		/ get data into ax
	pop	%bp
	lret			/ return

	.globl	inb
inb:	push	%bp		/ save bp
	mov	%sp,%bp		/ get the base of parameters
	mov	6(%bp),%dx	/ get port number
	inb	(%dx)		/ get data in to ax
	movb	$0,%ah		/ zero the high byte
	pop	%bp
	lret			/ return

	.globl	out
out:	push	%bp		/ save bp
	mov	%sp,%bp		/ get the base of parameters
	mov	6(%bp),%dx	/ get port number
	mov	8(%bp),%ax	/ get data
	out	(%dx)		/ put it out
	pop	%bp
	lret			/ return

	.globl	outb
outb:	push	%bp		/ save bp
	mov	%sp,%bp		/ get the base of parameters
	mov	6(%bp),%dx	/ get port number
	mov	8(%bp),%ax	/ get data
	outb	(%dx)		/ put it out
	pop	%bp
	lret			/ return

	.globl	outb2
outb2:	push	%bp		/ save bp
	mov	%sp,%bp		/ get the base of parameters
	mov	6(%bp),%dx	/ get port number
	mov	8(%bp),%ax	/ get data
	outb	(%dx)		/ put it out
	mov	10(%bp),%ax	/ get data byte 2
	outb	(%dx)		/ put it out
	pop	%bp
	lret			/ return

	.globl	outb21
outb21:	push	%bp		/ save bp
	mov	%sp,%bp		/ get the base of parameters
	mov	6(%bp),%dx	/ get port number
	mov	8(%bp),%ax	/ get data
	outb	(%dx)		/ put it out
	inc	%dx		/ bump to port + 1
	mov	10(%bp),%ax	/ get data byte 2
	outb	(%dx)		/ put it out
	pop	%bp
	lret			/ return


/
/ reloadregs
/	force the 286 to reload various selector registers
/	and all the invisible associated registers.
/	(i.e. limit, base, and access)
/	we have to be real tricky with the stack
/
	.globl	reloadregs
reloadregs:
	sldt	%ax		/ get ldt selector
	lldt	%ax		/ put it back, ( loads the invisible regs also )
	str	%ax		/ get tss selector
	ltr	%ax		/ put it back, ( loads the invisible regs also )
				/ and turns on the busy bit
	mov	%bp,%si		/ save frame pointer
	mov	%sp,%bp		/ point at return address
	mov	%ss,%dx		/ get stack selector
	mov	%dx,%es		/ load %es with the new invisible regs
	mov	0(%bp),%ax	/ move offset from old stack ...
	mov	%ax,%es:0(%bp)	/ ... to new stack
	mov	2(%bp),%ax	/ move selector from old stack ...
	mov	%ax,%es:2(%bp)	/ ... to new stack
	mov	%dx,%ss		/ put stack selector back (load invisible regs)
	mov	%si,%bp		/ restore frame pointer
	lret

/
/ reloadptr() reloads the task register
/ with the parent process' tss gate selector
/
	.globl	reloadptr
reloadptr:
	push	%bp
	mov	%sp,%bp
	mov	6(%bp),%ax
	ltr	%ax
	pop	%bp
	lret

