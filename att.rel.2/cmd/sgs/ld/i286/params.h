/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)params.h	1.4 - 85/09/03 */

/*
 *   This file contains system dependant parameters for the link editor.
 *   Normally, these are #define constants, but for the iapx loader they
 *   must be global variables.
 */


/*
 * Maximum size of a section
 */
extern long MAXSCNSIZE;
extern long MAXSCNSZ;


/*
 * Default size of configured memory
 */
#if iAPX286
#define SMTXTORG	0x570000L
#define NEXTSECTION	0x80000L
#define MEMORG		0x0L
#else
extern long MEMORG;
#endif
extern long MEMSIZE;

/*
 * Size of a region. If USEREGIONS is zero, the link editor will NOT
 * permit the use of REGIONS, nor partition the address space.
 * USEREGIONS is defined in system.h
 */
extern long REGSIZE;

extern unsigned short dynamagic;

extern void	 initvars();		/* This routine initializes the
					 * "parameters" normally found in
					 * params.h
					 */
#define TVMAGIC (dynamagic+1)	/*
				 * note: dynamagic is a global variable
				 * which is assigned a value based
				 * on the magic number of the object
				 * file in the initvars procedure.
				 */

/*
 *	When a UNIX aout header is to be built in the optional header,
 *	the following magic numbers can appear in that header:
 *
 *		AOUT1MAGIC : default
 */

#define AOUT1MAGIC 0410
#define AOUT2MAGIC 0407


#if iAPX286
/*
 * define a local flag that special1.c must deal with
 */

extern short kflag,	/* user wants to change the stack size from STKSIZ */
	     Kflag,	/* user wants actual byte counts in the unix header */
	     Rflag;	/* user wants real address mode - not protected mode */

/* 
 * define the default stack size for small model
 */

#define STKSIZ		0x2000

/*
 * define the "model" types
 */

#define M_SMALL		1
#define M_LARGE		2
#define M_HUGE		4

#endif /* if iAPX286 */

/*
 * define alignment for common symbols
 */

#define COM_ALIGN 	0x1L

/*
 * define special symbol names
 */

#define _CSTART	"_start"
#define _MAIN	"main"
