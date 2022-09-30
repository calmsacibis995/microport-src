/* uportid = "@(#)text.h	Microport Rev Id  1.3.3 6/18/86" */
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)text.h	1.4 */
/*
 * Text structure.
 * One allocated per pure procedure on swap device.
 * Manipulated by text.c
 */
struct text
{
	ushort	x_daddr;	/* disk address of segment (relative to swplo) */
	ushort	x_size;		/* size (clicks) */
	ushort x_caddr;		/* address of text (clicks) */
	struct inode *x_iptr;	/* inode of prototype */
	char	x_count;	/* reference count */
	char	x_ccount;	/* number of loaded references */
	char	x_flag;		/* traced, written flags */
};

extern struct text text[];

#define	XTRC	01		/* Text may be written, exclusive use */
#define	XWRIT	02		/* Text written into, must swap out */
#define	XLOAD	04		/* Currently being read from file */
#define	XLOCK	010		/* Being swapped in or out */
#define	XWANT	020		/* Wanted for swapping */
