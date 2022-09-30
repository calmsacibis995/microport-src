/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#define SGS     ""

/*	The MAGIC number constants are defined in filehdr.h	*/


#define I286MAGIC(x)	(((unsigned short) x) == ((unsigned short) I286LMAGIC) || ((unsigned short) x) == ((unsigned short) I286SMAGIC))

#define ISMAGIC(x)	( I286MAGIC(x) )

#ifdef ARTYPE
#define	ISARCHIVE(x)	(((unsigned short) x) == (unsigned short) ARTYPE)
#define BADMAGIC(x)	((((x) >> 8) < 7) && !ISMAGIC(x) && !ISARCHIVE(x))
#endif


#define SGSNAME ""
#define RELEASE "System V Release 2.0 10/1/85"
