/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)slotvec.h	1.3 - 85/08/09 */


struct slotvec{
	long svsymndx;		/* original (input) symbol table index	*/
	long svovaddr;		/* original (input) symbol virtual addr	*/
	long svnvaddr;		/* new (output) symbol virtual addr	*/
	long svnewndx;		/* new (output) symbol table index	*/
	char svsecnum;		/* new (output) section number		*/
	char svflags;
	};

#define SLOTVEC struct slotvec
#define SLOTSIZ sizeof(SLOTVEC)

#if TRVEC
#define	SV_TV		01
#define	SV_ERR		02
#else
#define SV_TV		0
#define SV_ERR		0
#endif

extern SLOTVEC	*svread( );
#define GETSLOT(slt,action)	&svpageptr[(slt)]
