/*
 * Defines for accessing the PC AT CMOS ram.
 */

/*
 * Copyrighted as an unpublished work.
 * (c) Copyright 1986 INTERACTIVE Systems Corporation
 * All rights reserved.
 *
 * RESTRICTED RIGHTS
 *
 * These programs are supplied under a license.  They may be used,
 * disclosed, and/or copied only as permitted under such license
 * agreement.  Any copy must contain the above copyright notice and
 * this restricted rights notice.  Use, copying, and/or disclosure
 * of the programs is strictly prohibited unless otherwise provided
 * in the license agreement.
 */

#ident "@(#)cram.h	1.3 - 87/01/06"

#define	CMOS_ADDR	0x70	/* I/O port address for CMOS ram address */
#define	CMOS_DATA	0x71	/* I/O port address for CMOS ram data */

#define	DSB		0x0e	/* Diagnostic status byte ram address */
#define	SSB		0x0f	/* Shutdown status byte ram address */
#define DDTB		0x10	/* Diskette drive type byte ram address */
#define	FDTB		0x12	/* Fixed disk type byte ram address */
#define	EB		0x14	/* Equipment byte ram address */
#define	BMLOW		0x15	/* Base mem size low byte ram address */
#define	BMHIGH		0x16	/* Base mem size high byte ram address */
#define	EMLOW		0x17	/* Expansion mem size low byte ram address */
#define	EMHIGH		0x18	/* Expansion mem size high byte ram address */
#define	DCEB		0x19	/* Drive C Extended byte ram address */
#define	DDEB		0x1a	/* Drive D Extended byte ram address */
#define	CKSUMLOW	0x2e	/* Checksum low byte ram address */
#define	CKSUMHIGH	0x2f	/* Checksum high byte ram address */
#define	EMLOW2		0x30	/* Expansion mem size low byte ram address */
#define	EMHIGH2		0x31	/* Expansion mem size high byte ram address */
#define	DCB		0x32	/* Date century byte ram address */
#define	IF		0x33	/* Information flag ram address */

/*
 * ioctls for accessing CMOS ram.
 */
#define CMOSIOC	('C' << 8)

#define	CMOSREAD	(CMOSIOC | 0x01)
#define	CMOSWRITE	(CMOSIOC | 0x02)

extern unsigned char	CMOSread();
