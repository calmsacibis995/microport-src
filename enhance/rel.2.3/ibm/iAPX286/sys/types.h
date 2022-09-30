/* uportid = "@(#)types.h	Microport Rev Id  1.3.3 6/18/86" */
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)types.h	1.3 */
typedef	struct { int r[1]; } *	physadr;
typedef	long		daddr_t;
typedef	char *		caddr_t;
typedef	unsigned int	uint;
typedef	unsigned short	ushort;
typedef	ushort		ino_t;
typedef short		cnt_t;
typedef	long		time_t;
typedef	int		label_t[10];
typedef	short		dev_t;
typedef	long		off_t;
typedef	long		paddr_t;
typedef	long		key_t;
