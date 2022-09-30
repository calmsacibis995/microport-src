.file "hdboot.s"
/	@(#)hdboot.s	2.3
/	sccid @(#)hdboot.s	1.8
/
/ hdboot.s:	The hard disk boot block for the IBM AT.
/	This is the boot block code (sometimes referred to as
/	"little boot") for the hard disk. It reads in the
/	bootstrap program that understands how to find /unix
/	(and is called "big boot", or "stlboot", or "coffboot").
/
/	The AT uses a partition table, residing at sector 1, track 0,
/	cylinder 0. One must first find the active partition before
/	one can load in the appropriate big boot.
/		uport!dwight	1/28/86
/
/ M000:		uport!dwight Sun Mar  9 15:29:33 PST 1986
/	Removed all hardwired drive parameters. This boot block
/	now gets drive info via rom bios, and passes it to big boot.
/ M001:		uport!larry  Wed Mar 12 1986
/	add support for badtrack mapping
/
/ M002:		uport!larry  Tue Apr 8 1986
/	add support for non-std drives
/
/ M003:		uport!dwight Mon May 12 08:41:41 PDT 1986
/	Changed infinite retries to finite retries.
/ M004:		uport!rex	4/28/87
/	Print the "boot (" part of the message "boot (/system5):"
/	upon startup
/	Add a drive parameter table to this boot sector that can be
/	loaded into memory and pointed to by the vector 0x104 for the
/	first unit.  This is to make sure the int 0x13 interface will
/	work even if the drive is not supported by the ROM table.
/	The code that loads the table is also in this program.
/	Also fixed a bug where this program only loaded 7 sectors
/	of Big Boot instead of 8.
/
#define	BOOTTRACK	1	/* Track where Big Boot lives		*/
#define	BOOTSECTOR	1	/* The starting sector of Big Boot	*/
#define	BOOTCOUNT	8	/* # of sectors read, of boot track	*/

#define	NOP .byte	0x90	/* Simulated nop; `as` doesn't have one!*/
#define	ESSEG		0x7a00	/* es: where big boot gets stuffed	*/

#define	STACK		0x8202	/* Area used as sp			*/
#define	BOOTDEV		0x80	/* 0/1: floppy. 0x80/0x81: harddisk	*/
#define	BBOOTSIZE	4096 	/* max size of big boot	= 8 * 512	*/
#define	BOOT_START	0x00A8	/* size of coffboot header		*/
#define NRETRIES	100	/* M003: avoid wear and tear		*/

				/* M004: define the source and
				 * destination offsets of the DPT.
				 * Source is this boot sector loaded at
				 * 0000:7C00 + offset withing sector	*/
#define	BSDPTOFF	0x7DEE	/* 0x7C00+494 Boot Sector DPT OFFset	*/
#define	NEWDPTOFF	0x3C4	/* destination OFFset of the NEW DPT	*/

	.text

stl_bootstrp_start:
	push	%ds			/ M004: save for later restore
	mov	$ESSEG,%ax		/ borrow this memory for a while
	mov	%ax,%ds
	mov	$0x00,%si
	movb	$0x62,0(%si)		/ 'b'
	movb	$0x6f,1(%si)		/ 'o'
	movb	$0x6f,2(%si)		/ 'o'
	movb	$0x74,3(%si)		/ 't'
	movb	$0x20,4(%si)		/ ' '
	movb	$0x28,5(%si)		/ '('
	movb	$0x00,6(%si)
	cld
next_char:
	movb	(%si),%al
	inc	%si
	pop	%ds			/ restore for jmp or int
	andb	$0x7f,%al		/ check for NULL terminator
	jz	retry_mboot_block
	mov	$7,%bx			/ page 0, attribute 7
	movb	$0xe,%ah		/ write teletype
	int	$0x10			/ video interface
	push	%ds
	mov	$ESSEG,%ax
	mov	%ax,%ds
	jmp	next_char		/ M004: end

retry_mboot_block:
	call	read_mboot_block	/* read the master boot block	*/
	jnz	halt			/* failed -> stop		*/

					/ M004:	START load for BSDPT
	mov	$BSDPTOFF,%si		/ get pointer to our drive table
	cmpb	$00,2(%si)		/ see if we have a table
	je	retry_pboot_eblock	/ none if # heads = 0
	push	%es			/ save es
	xor	%ax,%ax
	mov	%ax,%es			/ make sure destination segment is 0x00
	mov	$NEWDPTOFF,%di		/ and the offset is in %di
	cld
	mov	$0x10,%cx		/ size of the table entry
	rep
	smovb				/ do the copy
	mov	$0x104,%di		/ the drive 0 param vector
	mov	$NEWDPTOFF,%es:(%di)	/ the table offset
	mov	$0x000,%es:2(%di)	/ the table segment
	pop	%es			/ restore es
	movb	$BOOTDEV,%dl		/ the disk unit #
	movb	$0x9,%ah		/  for the reset command
	int	$0x13			/  using the new DPT
					/ Now all set for disk I/O
					/ M004:	END
retry_pboot_eblock:
	call	read_pboot_eblock
	call	gethdparams		/ M000: get boot device info

	mov	$STACK,%sp		/ establish stack
	mov	$0x0000,%bx
	mov	headdrive,%dx		/ M000: only want max head, in dh
	movb	$BOOTDEV,%dl		/ dx now refers to last track on part.
	movb	peblock_cyl,%ch		/ cyl
	movb	peblock_sector,%cl
	andb	$0xC0,%cl		/ top 2 bits, for end cyl
	orb	$0x01,%cl		/ 1st sector (of last track)
	movb	$0x01,%al		/ one sector at a time
	movb	$0x02,%ah		/ read command

	push	%ax			/ save
/	push	%bx
	mov	$ESSEG,%ax		
	mov	%ax,%es			/ es:ESSEG => where big boot goes
/	pop	%bx			/ restore
	pop	%ax

try_bootstrp:
	push	%ax			/ save for int 13h call
	mov	$retries,%ax		/ M003
	cmp	$0,%ax			/ M003
	jne	retry_bootstrp		/ M003
halt:					/ M003: She's suckin' mud,
	sti				/ M003: so shut 'er down, scotty.
	hlt				/ M003
	jmp	halt			/ M003

retry_bootstrp:				/ M003
	dec	%ax			/ M003
	mov	%ax,retries		/ M003
	pop	%ax			/ M003
	push	%ax			/ M003
	
	push	%bx
	push	%cx
	push	%dx
	push	%es

	int	$0x13			/ call the fixed disk i/o interface
	pop	%es			/ recover
	pop	%dx
	pop	%cx
	pop	%bx
	pop	%ax
	jc	try_bootstrp		/ on error, infinite retries
	incb	%cl			/ increment sector count
	push	%cx			/ and save it for next pass
	andb	$0x0F,%cl
	cmpb	$BOOTCOUNT,%cl		/ sector count = Boot size?
	pop	%cx
	jbe	no_new_track		/ no => +512 and try again / M004:
	jmp	boot_jump		/ loaded, so try jumping to it,
	NOP				/ flush prefetch queue

no_new_track:
	add	$512,%bx
	jmp	try_bootstrp

boot_jump:				/ prepare for liftoff
	push	%ds			/ save data segment we've been using
	push	%es			/ save Big Boot segment
	pop	%ds
	mov	$BOOT_START,%si		/ offset into Big Boot (header size)
	mov	$0x0000,%di
	cld
	mov	$BBOOTSIZE,%cx		/ size of big boot, in 512 blocks
	rep				/ copy, stripping header 
	smovb

	pop	%ds			/ back to the old ds (superflous)
/ the following parameters get passed to big boot:
/ peblock_cyl,peblock_sector,peblock_head: disk address of partition
/			end record in active partition.		M001
/ headdrive:	max # of heads , boot device # (80/81 for harddisk)
/ maxcylsec:	max cyl #, and # of sectors per track.
/ pbblock_cyl:	starting cylinder # of the active partition.
/ pbblock_sector: starting sector # of the active partition.

	movb	peblock_head,%cl	/ M001
	push	%cx			/ M001
	movb	peblock_cyl,%ch		/ M001
	movb	peblock_sector,%cl	/ M001
	push	%cx			/ M001
	mov	headdrive,%cx		/ M000
	movb	$BOOTDEV,%cl		/ M000
	push	%cx			/ M000
	mov	maxcylsec,%cx		/ M000
	push	%cx			/ M000
	movb	pbblock_cyl,%ch		/ one last pick up
	movb	pbblock_sector,%cl
	push	%cx			/ pass it to Big Boot
	push	%es
	pop	%ds			/ ds = future cs
	push	%es
	mov	$0x0000,%dx
	push	%dx
	lret				/ airborne, we hope

read_mboot_block:
	mov	$ESSEG,%ax
	mov	%ax,%es
	mov	$0x0201,%ax
	mov	$0x0000,%bx
	mov	$0x0001,%cx
	mov	$0x0080,%dx
	int	$0x13
/	jc	read_mboot_block	/ M003

	cld
	mov	$0x01BE,%si
	mov	$0x0004,%cx
	
look4_actpart:
	slodb	%es
	cmpb	$0x80,%al
	jne	part_notact

	movb	%es:2(%si),%al
	movb	%al,pbblock_cyl
	movb	%es:0(%si),%al
	movb	%al,pbblock_head
	movb	%es:1(%si),%al
	movb	%al,pbblock_sector
	movb	%es:6(%si),%al
	movb	%al,peblock_cyl		/* es:[si]+6			*/
	movb	%es:4(%si),%al
	movb	%al,peblock_head	/* es:[si]+4			*/
	movb	%es:5(%si),%al
	movb	%al,peblock_sector	/* es:[si]+5			*/
	inc	%dx

part_notact:
	add	$0x000F,%si
	loop	look4_actpart
	cmp	$0x0081,%dx		/ 81 = 80 + 1 (inc dx)
	ret

read_pboot_eblock:
	mov	$ESSEG,%ax
	mov	%ax,%es
	mov	$0x0201,%ax		/ read 1 sector 
	mov	$0x0000,%bx		/ into es:bx
	movb	peblock_sector,%cl
	movb	peblock_cyl,%ch
	movb	peblock_head,%dh
	movb	$0x80,%dl		/ #0 harddisk
	int	$0x13
/	jc	read_pboot_eblock	/ infinite retries M003
	mov	$0x01BE,%si		/ where partition info lives

	movb	%es:1(%si),%al
	movb	%al,stl_boot_head	/ es:[si]+1
	movb	%es:2(%si),%al
	movb	$0x01,stl_boot_sec	/ es:[si]+2: always 1
	movb	%es:3(%si),%al
	movb	%al,stl_boot_cyl	/ es:[si]+3
	ret

/ get the bootdevice's parameters
/	(implied by partition end block).	/ start M002
gethdparams:
	movb	peblock_sector,%cl
	movb	peblock_cyl,%ch
	movb	peblock_head,%dh
	movb	$0x80,%dl		/ #0 harddisk
	mov	%dx,headdrive		/ save for big boot
	mov	%cx,maxcylsec		/ like ditto
	ret				/ end M002

/* superfluous */
/ read_pboot_bblock:
/ 	mov	$ESSEG,%ax
/ 	mov	%ax,%es
/ 	mov	$0x0201,%ax
/ 	mov	$0x0000,%bx
/ 	movb	pbblock_sector,%cl
/ 	movb	pbblock_cyl,%ch
/ 	movb	pbblock_head,%dh
/ 	movb	$0x80,%dl
/ 	int	$0x13
/	jc	read_pboot_bblock		/ M003
/ 	ret

pbblock_cyl:	.byte	0
pbblock_head:	.byte	0
pbblock_sector:	.byte	0

peblock_cyl:	.byte	0
peblock_head:	.byte	0
peblock_sector:	.byte	0

stl_boot_cyl:	.byte	0
stl_boot_head:	.byte	0
stl_boot_sec:	.byte	0

headdrive:	.value	0x0000
maxcylsec:	.value	0x0000
retries:	.value	NRETRIES		/* M003 */

					/ M004: the DPT entry for boot disk
	. = 494^.
	.value	0x0000			/ cylinders
	.byte	0x00			/ heads
	.value	0x0000			/ not used
	.value	0x0000			/ write precomp
	.byte	0x00			/ max ECC burst length
	.byte	0x00			/ control byte
	.byte	0x00			/ not used
	.byte	0x00			/ not used
	.byte	0x00			/ not used
	.value	0x0000			/ landing zone
	.byte	0x00			/ sectors per track
	.byte	0x00			/ reserved

	. = 510^.
	.value	0xaa55	 /	aa55 at 510d = boot block validation
