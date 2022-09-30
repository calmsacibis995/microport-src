/* uportid = "@(#)init.h	Microport Rev Id  1.3.3 6/18/86" */
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */
/*
 * M000 lance 3-10-86
 *	initpic() is called explicity at beginning, instead of through here
 */

/* @(#)init.h	1.6 */
extern int clkstart(),cinit(),binit(),iinit(),inoinit();
/* extern int initpic();  M000 */
extern int finit();
extern int flckinit();
#ifdef X25_0
extern x25init();
#endif
#ifdef BX25S_0
extern bxncinit();
extern sessinit();
#endif
#ifdef ST_0
extern	stinit();
#endif
#ifdef	VPM_0
extern	vpminit();
#endif
#ifdef EM_0
extern  eminit();
#endif

/*	Array containing the addresses of the various initializing	*/
/*	routines executed by "main" at boot time.			*/

int (*init_tbl[])() = {
	inoinit,
/*	initpic,  M000 */
	clkstart,
	cinit,
	binit,
	finit,
	iinit,
	flckinit,
#ifdef	VPM_0
	vpminit,
#endif
#ifdef X25_0
	x25init,
#endif
#ifdef ST_0
	stinit,
#endif
#ifdef BX25S_0
        bxncinit,
        sessinit,
#endif
#ifdef EM_0
	eminit,
#endif
	0
};
