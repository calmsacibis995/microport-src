.file "flboot.s"
/
/ flboot.s:	The floppy boot block for the IBM AT.
/	This is the boot block code (sometimes referred to as
/	"little boot") for the floppy device. It reads in the
/	bootstrap program that understands how to find /unix
/	(and is called "big boot", or "stlboot", or "coffboot").
/	Big boot is assumed to live on the track BOOTTRACK, starting
/	at sector BOOTSECTOR.
/
/		uport!dwight	12/29/85
/
/ M000:	uport!dwight 2/14/86
/	Made BOOTSTART = 0x00A8, for coff big boot
/ M001:		uport!dwight Mon May 12 08:41:41 PDT 1986
/	Changed infinite retries to finite retries.

#define	BOOTTRACK	1	/* Track where Big Boot lives		*/
#define	BOOTSECTOR	1	/* The starting sector of Big Boot	*/
#define	BOOTDEVICE	0	/* The device that contains Big Boot	*/
#define	BOOTSTART	0x00A8	/* M000 Header length, to executable code */
#define	BOOTCOUNT	8	/* # of sectors read, of boot track	*/

#define	NOP .byte	0x90	/* Simulated nop; `as` doesn't have one!*/
#define	STACK		0x8202	/* Area used as sp			*/
#define	ESSEG		0x7a00	/* es: segment				*/

#ifdef	HIDENSITY
#define	DISKTYPE	0x03	/* hi dens disk, hi dens drive	*/
#else
#define	DISKTYPE	0x02	/* low dens disk, hi dens drive	*/
#endif	HIDENSITY

#define NRETRIES	100	/* M001: avoid wear and tear		*/

	.text

start:
	mov	$STACK,%sp		/* Establish stack pointer	*/
	mov	$0x0000,%bx		/* buffer offset from es:	*/
	movb	$BOOTDEVICE,%dl		/* Set up device to be used	*/
	movb	$BOOTTRACK,%dh		/* Track to be loaded in	*/
	andb	$0x01,%dh
	movb	$BOOTSECTOR,%ch		/* Starting sector on boottrack	*/
	shr	$0x01,%cx
	movb	$BOOTSECTOR,%cl
	movb	$0x01,%al		/* read 1 sector at a time	*/
	movb	$0x02,%ah
	push	%ax
	push	%bx
	mov	$ESSEG,%ax		/* set up es:			*/
	jmp	bootstrap			
halt:					/* M001				*/
	NOP				/* Flush prefetch queue		*/
	sti				/* error, so die		*/
	hlt
	jmp	halt			/* M001				*/

bootstrap:				/* start loading in big boot	*/
	mov	%ax,%es			/* establish es:		*/
	push	%ax			/* M001: save for later		*/
	mov	$retries,%ax		/* M001				*/
	cmp	$0,%ax			/* M001				*/
	jne	retry_bootstrp		/* M001				*/
	jmp	halt			/* M001				*/

retry_bootstrp:				/* M001				*/
	dec	%ax			/* M001				*/
	mov	%ax,retries		/* M001				*/
	pop	%ax			/* M001 recover			*/

	push	%es
	push	%dx
	push	%cx
					/* read dasd type, via ROM BIOS	*/
	movb	$0x15,%ah		/* floppy command, for read dasd*/
	int	$0x13
	jc	start			/* M001				*/
					/* set dasd type, via ROM BIOS	*/
	movb	$DISKTYPE,%al		/* low or hi dens disk 		*/
	movb	$0x00,%dl		/* 0 = top drive, 1 = bottom 	*/
	movb	$0x17,%ah		/* set dasd type, via ROM BIOS	*/
	int	$0x13			/* call ROM BIOS floppy driver	*/
	jc	start			/* M001				*/

	pop	%cx
	pop	%dx
	pop	%es
	pop	%bx

	mov	$NRETRIES,%ax		/* M001: reinit			*/
	mov	%ax,retries		/* M001				*/

	pop	%ax

readsector:
	push	%ax			/* ah=1 -> read, al=# of sectors*/

	mov	$retries,%ax		/* M001				*/
	cmp	$0,%ax			/* M001				*/
	je	halt			/* M001				*/

retry_read:				/* M001				*/
	dec	%ax			/* M001				*/
	mov	%ax,retries		/* M001				*/
	pop	%ax			/* M001 recover			*/
	push	%ax			/* M001				*/

	push	%bx			/* buffer offset from es:	*/
	push	%cx			/* cl = sec #, ch = track #	*/
	push	%dx			/* dl = drive #, dh = head #	*/
	push	%es			/* buffer segment address	*/
	int	$0x13			/* read sector, via ROM BIOS	*/
	pop	%es			/* restor values		*/
	pop	%dx
	pop	%cx
	pop	%bx
	pop	%ax
	jc	readsector		/* on error, infinite retries	*/

	incb	%cl			/* want next sector		*/
	cmpb	$0x09,%cl		/* if this is the last sector	*/
	jc	label3
	.value	0x07eb
	NOP
label3:
	add	$0x0200,%bx
	.value	0xe2eb
	push	%ds
	push	%es
	pop	%ds
	mov	$BOOTSTART,%si
	mov	$0x0000,%di
	cld
	mov	$0x1000,%cx
	repz
	smovb
	pop	%ds
	push	%es
	mov	$0x0000,%dx
	push	%dx
	lret

retries:	.value	NRETRIES		/* M001 */

	. = 510^.
	.value	0xaa55	 /	aa55 at 510d = boot block validation
