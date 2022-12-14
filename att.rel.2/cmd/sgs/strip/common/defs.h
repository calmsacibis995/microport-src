/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)defs.h	1.3 - 85/08/09 */
#define	ON	1
#define	OFF	0

#define FATAL	1
#define ERROR	-999L

#define	BSIZE	256
#define MAXSECTIONS	256
#define MAXTEMPENTS	1024

#define	TEMPENT		struct tempent
struct	tempent
{
	long	new;
	long	old;
#if !AR16WR
	TEMPENT	*link;
#endif
};

#define	TEMPSZ		sizeof(TEMPENT)

