/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)seg.h	1.10 */
/*
 * Memory management structures
 */

/*
** Access Abilities
*/
#define	RO	ACC_UCODE
#define	RW	ACC_UDATA

/*
** Segment Descriptor structure
*/
struct	seg_desc
{
	ushort		sd_limit;	/* limit of segment		*/
	ushort		sd_lowbase;	/* bits 0-15 of base of seg	*/
	unsigned char	sd_hibase;	/* bits 16-23 of base of seg	*/
	unsigned char	sd_access;	/* access byte			*/
	ushort		sd_reserved;	/* must be zero for 386 compat	*/
};

/*
** Gate Descriptor structure
*/
struct	gate_desc
{
	ushort		gd_offset;	/* destination offset		*/
	ushort		gd_sel;		/* destination selector		*/
	unsigned char	gd_wcount;	/* word count			*/
	unsigned char	gd_access;	/* access byte			*/
	ushort		gd_reserved;	/* must be zero for 386 compat	*/
};

/*
** Control Descriptors for Process Slot Packets
*/
struct	packet
{
	struct seg_desc	pa_tss_d,	/* TSS descriptor	*/
			pa_tss_ad,	/* TSS alias descriptor	*/
			pa_ldt_d,	/* LDT descriptor	*/
			pa_ldt_ad;	/* LDT alias descriptor	*/
};

/*
** Array of Process Slot Packets indexed by process table index
*/
extern struct packet	pslot[];

/*
** Array of Segment Descriptors indexed by global descriptor table index
*/
extern struct seg_desc	gdt[];

