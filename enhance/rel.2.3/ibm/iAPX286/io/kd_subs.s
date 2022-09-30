	.file	"kd_subs.s"
	.ident	"@(#)kd_subs.s	3.0 Microport Rev 2.3 10/1/87"
	.data
	.text
	.def	kdasput; .val kdasput; .scl 2; .type 044; .endef
	.set	.T1,1
	.set	.S1,0

#include "io_asm.h"

#define	ATRBREG	0xC0
#define	ENAPAL	0x20
#define NOP 	.byte 0x90
#define	VIDOFF	0xF7

/* kdasput (reg, adrs, data, mode) */
	.set	P2,[P1 + PORT]
	.set	P3,[P2 + 4]
	.set	P4,[P3 + INT]
	.globl	kdasput
kdasput:
	enter	$0,$0
	PUSHDI
	movw	P1(BP),%dx	/* mode reg address */
	addw	$2,%dx		/* up to status reg */
	LES	P2(BP),DI	/* target address */
	MOVINT	$LOOPCNT,CX	/* loop timeout */
kdasputloop:
	sti
	NOP
	cli	
	NOP
	inb	(%dx)
	testb	$1,%al
	loope	kdasputloop
	movw	$1,%ax
	movw	P1(BP),%dx
	outb	(%dx)
	movw	P3(BP),%ax
	movw	%ax,ESI(DI)
	movw	P4(BP),%ax
	outb	(%dx)
	sti
	POPDI
	leave	
	RET	

/* wait for horizontal retrace - returns with interrupts OFF!
 * kd_hwait (port)
 * Since this is used for *short* accesses to the screen buffer,
 * this routine waits only for the presence of the regen buffer available
 * status bit. The timing window is short, because we could be
 * just at the end of the retrace.
 */
	.globl	kd_hwait
kd_hwait:
	enter	$0,$0
	movw	P1(BP),%dx	/* address of status port	*/
	MOVINT	$LOOPCNT,CX	/* loop timeout			*/
kd_hw1:	sti			/* interrupts on		*/
	NOP
	cli			/* interrupts off		*/
	inb	(%dx)		/* get status			*/
	testb	$1,%al		/* check for horizontal retrace	*/
	jz	kd_hw1		/* loop until not there		*/
	leave	
	RET	

/* wait for vertical retrace, returns with ints OFF */
vertwt: MOVINT	$LOOPCNT,CX	/* loop timeout			*/
vertw1:	inb	(%dx)		/* get status			*/
	testb	$8,%al		/* check for vertical retrace	*/
	loopne	vertw1		/* loop until not there		*/
	MOVINT	$LOOPCNT,CX	/* loop timeout			*/
vertw2:	sti			/* interrupts on		*/
	NOP
	cli			/* interrupts off		*/
	inb	(%dx)		/* get status			*/
	testb	$8,%al		/* check for vertical retrace	*/
	loope	vertw2		/* loop until there		*/
	ret			/* return to caller		*/

/* kd_vidoff (port, mode) */
	.set	P2,[P1 + PORT]
	.globl	kd_vidoff
kd_vidoff:
	enter	$0,$0
	movw	P1(BP),%dx
	addw	$6,%dx		/* address of status port	*/
	call	vertwt		/* wait for vertical retrace	*/
	movw	P2(BP),%ax
	andw	$VIDOFF,%ax	/* turn off video enable	*/
	subw	$2,%dx		/* down to mode reg		*/
	outb 	(%dx)
	sti			/* interrupts on		*/
	leave	
	RET	

/* wait for vertical retrace - returns with interrupts OFF!
 * kd_vwait (port)
 * Since this routine is used for *long* accesses, we wait for the
 * absence of vertical retrace and then the leading edge of vertical
 * retrace. This guarantees us the longest available time for
 * accessing the screen memory and controller registers.
 */
	.globl	kd_vwait
kd_vwait:
	enter	$0,$0
	movw	P1(BP),%dx	/* address of status port	*/
	call	vertwt		/* wait for vertical retrace	*/
	leave	
	RET	

/* move words backwards - kd_movwb (base, to, offset, cnt, flkrreg, modereg) */
/*				      6   10   12     14   16	    18       */
	.set	P2,[P1 + 4]
	.set	P3,[P2 + INT]
	.set	P4,[P3 + INT]
	.set	P5,[P4 + INT]
	.set	P6,[P5 + PORT]
	.globl	kd_movwb
kd_movwb:
	enter	$0,$0
	PUSHSI
	PUSHDI
	LES	P1(BP),DI	/* figure destination */
	movw	P2(BP),%ax
	addw	%ax,%ax
	addw	%ax,DI
	LDS	P1(BP),SI	/* figure source */
	addw	%ax,SI
	movw	P3(BP),%ax
	addw	%ax,%ax
	addw	%ax,SI
	movw	P5(BP),%dx	/* get flicker reg */
	testw	%dx
	jz	kd_mb2		/* not set */
	addw	$6,%dx		/* point to status reg */
	call	vertwt		/* wait for vertical retrace	*/
	movw	P6(BP),%ax	/* get mode reg */
	andw	$VIDOFF,%ax	/* turn off video enable */
	subw	$2,%dx		/* down to mode reg */
	outb	(%dx)		/* turn off video */
	sti			/* interrupts on */
kd_mb2:	MOVINT	P4(BP),CX	/* get count */
	rep
	smov	DSI(SI),ESI(DI)
	testw	%dx		/* flicker problem ? */
	jz	kd_mb3		/* no */
	movw	P6(BP),%ax	/* get mode reg contents */
	outb	(%dx)		/* turn back on video */
	sti			/* interrupts on */
kd_mb3:	leave	
	POPDI
	POPSI
	RET	

/* move words forwards - kd_movwf (base, from, offset, cnt, flkrreg, modereg) */
/*				      6   10   12      14   16       18	*/
	.set	P2,[P1 + 4]
	.set	P3,[P2 + INT]
	.set	P4,[P3 + INT]
	.set	P5,[P4 + INT]
	.set	P6,[P5 + PORT]
	.globl	kd_movwf
kd_movwf:
	enter	$0,$0
	PUSHSI
	PUSHDI
	MOVINT	P4(BP),CX	/* get count */
	dec	CX		/* (-1) */
	LDS	P1(BP),SI	/* figure source */
	movw	P2(BP),%ax
	addw	%ax,%ax
	addw	%cx,%ax
	addw	%cx,%ax
	addw	%ax,SI
	LES	P1(BP),DI	/* figure destination */
	addw	%ax,DI
	movw	P3(BP),%ax
	addw	%ax,%ax
	addw	%ax,DI
	inc	CX		/* back to original count */
	std			/* direction backwards */
	movw	P5(BP),%dx	/* get flicker reg */
	testw	%dx		/* flicker problem ? */
	jz	kd_mf2		/* no */
	addw	$6,%dx		/* point to status reg */
	call	vertwt		/* wait for vertical retrace	*/
	movw	P6(BP),%ax
	andw	$VIDOFF,%ax	/* turn off video enable	*/
	subw	$2,%dx		/* down to mode reg */
	outb	(%dx)		/* turn off video */
	sti			/* interrupts on */
	MOVINT	P4(BP),CX	/* reget count (vertwt messed it up) */
kd_mf2:	rep
	smov	DSI(SI),ESI(DI)
	cld			/* direction back to forwards */
	testw	%dx		/* flicker problem ? */
	jz	kd_mf3		/* no */
	movw	P6(BP),%ax	/* get mode reg contents */
	outb	(%dx)		/* turn back on video */
kd_mf3:	
	POPDI
	POPSI
	leave	
	RET	

/* fill words - kd_fillw (fill, base, offset, cnt, flkrreg, modereg)	*/
/*			   6     8     12     14   16	    18	*/
	.set	P2,[P1 + INT]
	.set	P3,[P2 + 4]
	.set	P4,[P3 + INT]
	.set	P5,[P4 + INT]
	.set	P6,[P5 + PORT]
	.globl	kd_fillw
kd_fillw:
	enter	$0,$0
	PUSHDI
	LES	P2(BP),DI	/* get base */
	movw	P3(BP),%ax	/* addw offset */
	addw	%ax,DI
	addw	%ax,DI
	movw	P5(BP),%dx	/* get flicker reg */
	testw	%dx
	jz	kd_f2		/* not set */
	addw	$6,%dx		/* point to status reg */
	call	vertwt		/* wait for vertical retrace	*/
	movw	P6(BP),%ax
	andw	$VIDOFF,%ax	/* turn off video enable	*/
	subw	$2,%dx		/* down to mode reg */
	outb	(%dx)		/* turn off video */
	sti			/* interrupts on */
kd_f2:	MOVINT	P4(BP),CX	/* get count */
	movw	P1(BP),%ax	/* get source data */
	rep
	ssto
	testw	%dx		/* flicker problem ? */
	jz	kd_f3		/* no */
	movw	P6(BP),%ax	/* get mode reg contents */
	outb	(%dx)		/* turn back on video */
kd_f3:
	POPDI
	leave	
	RET	

/* bclr (adrs, cnt) */
	.globl	bclr
	.set	P2,[P1 + 4]
bclr:
	enter	$0,$0
	PUSHDI
	LES	P1(BP),DI	/* get target */
	MOVINT	P2(BP),CX	/* get count */
	movb	$0,%al		/* fill with zeroes */
	rep
	sstob
	POPDI
	leave
	RET

/* put gfx bits, read first then write	*/
/* putgfx (from, to, cnt);		*/
	.set	P2,[P1 + 4]
	.set	P3,[P2 + 4]
	.globl	kd_putgfx
kd_putgfx:
	enter	$0,$0
	PUSHSI
	PUSHDI
	LDS	P1(BP),SI		/* get source */
	LES	P2(BP),DI		/* get destination */
	MOVINT	P3(BP),CX		/* get count */
kd_pl:	movb	ESI(DI),%al		/* read to latch bits */
	smovb	DSI(SI),ESI(DI)		/* and copy */
	loop	kd_pl			/* loop until done */
	POPDI
	POPSI
	leave	
	RET	

/* kd_egaoff (statreg_addr)	*/
	.globl	kd_egaoff
kd_egaoff:
	enter	$0,$0
	movw	P1(BP),%dx	/* get stat reg address */
	MOVINT	$LOOPCNT,CX	/* loop timeout			*/
kd_e1:	inb	(%dx)
	testb	$8,%al		/* wait for vertical retrace */
	loopz	kd_e1
	movb	$ATRBREG,%dl	/* attribute reg	*/
	movb	$0,%al		/* turn off video	*/
	outb	(%dx)
	leave	
	RET	

/* kd_putmode (statreg_addr, index, data, video-on) */
	.set	P2,[P1 + PORT]
	.set	P3,[P2 + INT]
	.set	P4,[P3 + INT]
	.globl	kd_putmode
kd_putmode:
	enter	$0,$0
	movw	P1(BP),%dx	/* get stat reg address */
	MOVINT	$LOOPCNT,CX	/* loop timeout			*/
kd_p1:	inb	(%dx)		/* get status		*/
	NOP
	testb	$8,%al		/* wait for vertical retrace */
	loopz	kd_p1
	movb	$ATRBREG,%dl	/* attribute reg	*/
	movb	P2(BP),%al	/* register index */
	outb	(%dx)
	movb	P3(BP),%al	/* data */
	outb	(%dx)
	movb	P4(BP),%al	/* get video on flag	*/
	testb	%al,%al		/* test it		*/
	jz	kd_p3		/* exit if off		*/
	movw	P1(BP),%dx	/* get stat reg address */
	inb	(%dx)		/* clear adrs reg	*/
	movb	$ATRBREG,%dl	/* attribute reg	*/
	movb	$ENAPAL,%al	/* enable pallette access */
	outb	(%dx)
kd_p3:	leave	
	RET	

/* kd_inb (port) */
	.globl	kd_inb
kd_inb:
	enter	$0,$0
	MOVINT	$0,AX		/* clear return */
	movw	P1(BP),%dx	/* get port number */
	inb	(%dx)		/* get data in to al */
	leave
	RET

/* kd_inb21 (port,reg) */
	.set	P2,[P1 + PORT]
	.globl	kd_inb21
kd_inb21:
	enter	$0,$0
	movw	P1(BP),%dx	/* get port number */
	movw	P2(BP),%ax	/* get reg */
	outb	(%dx)		/* put it out */
	inc	DX		/* bump to port + 1 */
	MOVINT	$0,AX		/* clear return */
	inb	(%dx)		/* get data in to al */
	leave
	RET

/* kd_outb (port, data) */
	.set	P2,[P1 + PORT]
	.globl	kd_outb
kd_outb:
	enter	$0,$0
	movw	P1(BP),%dx	/* get port number */
	movw	P2(BP),%ax	/* get data */
	outb	(%dx)		/* put it out */
	leave
	RET

/*	kd_outb2 (port, byte, byte2)
 *		int port;
 *		char byte;		byte  -> (port)
 *		char byte2;		byte2 -> (port)
 */
	.set	P2,[P1 + PORT]
	.set	P3,[P2 + INT]
	.globl	kd_outb2
kd_outb2:
	enter	$0,$0
	movw	P1(BP),%dx	/* get port number */
	movw	P2(BP),%ax	/* get data */
	outb	(%dx)		/* put it out */
	movw	P3(BP),%ax	/* get data byte 2 */
	outb	(%dx)		/* put it out */
	leave
	RET

/*	kd_outb21 (port, byte, byte2)
 *		int port;
 *		char byte;		byte  -> (port)
 *		char byte2;		byte2 -> (port + 1)
 */
	.set	P2,[P1 + PORT]
	.set	P3,[P2 + INT]
	.globl	kd_outb21
kd_outb21:
	enter	$0,$0
	movw	P1(BP),%dx	/* get port number */
	movw	P2(BP),%ax	/* get data */
	outb	(%dx)		/* put it out */
	inc	DX		/* bump to port + 1 */
	movw	P3(BP),%ax	/* get data byte 2 */
	outb	(%dx)		/* put it out */
	leave
	RET

/*	kd_outb23 (port, reg, data)
 *		int port;		reg           -> port
 *		int reg;		hibyte (data) -> port + 1
 *		int data;		reg + 1       -> port
 *					lobyte (data) -> port + 1
 */
	.set	P2,[P1 + PORT]
	.set	P3,[P2 + INT]
	.globl	kd_outb23
kd_outb23:
	enter	$0,$0
	movw	P1(BP),%dx	/* get port number */
	movw	P2(BP),%ax	/* get reg */
	outb	(%dx)		/* output reg select */
	movb	P3+1(BP),%al	/* get hibyte (data) */
	inc	DX		/* bump to port + 1 */
	outb	(%dx)		/* output reg data */
	movw	P2(BP),%ax	/* get reg */
	inc	AX		/* bump to reg + 1 */
	dec	DX		/* back to original port */
	outb	(%dx)		/* output reg select */
	inc	DX		/* bump to port + 1 */
	movb	P3(BP),%al	/* get lobyte (data) */
	outb	(%dx)		/* output reg data */
	leave
	RET

	.def	kdasput; .val .; .scl -1; .endef
	.set	.F1,0
	.data
	.text
