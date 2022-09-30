/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)special1.c	1.3 - 85/08/09 */

#include <stdio.h>

#include "system.h"
#include "params.h"

#if AR32W
#define SWAPB2(x) swapb2(x)
#else
#define SWAPB2(x) x
#endif


#if TRVEC
void
chktvorg(org, tvbndadr)

long	org, *tvbndadr;
{

	/*
	 * check user-supplied .tv origin for legality
	 *  if illegal, side-effect tvspec.tvbndadr
	 *   and issue warning message
	 */

			if( (org & 0xf) != 0 )
				yyerror("tv origin (%10.0lx) must be a multiple of 16", org);
			*tvbndadr = (org + 0xfL) & ~0xfL;
}
#endif

void

#if iAPX286
extern char **argptr;
extern int argcnt;

specflags(flgname,ifile)
char *flgname;
int ifile;
#else
specflags(flgname, argptr)
char *flgname;
char **argptr;
#endif
{

	/*
	 * process special flag specification for Intel chips.
	 * These flags have fallen through switch of argname in ld00.c
	 */

	switch ( *flgname ) {
#if iAPX286
		case 'K':	/* the sizes in the unix header should be
				 * actual byte counts rather than sizes
				 * which have been rounded up to the nearest
				 * "click" size.  This option is provided to
				 * support the operating system people, and
				 * is expected to be used by them when using
				 * the large model programs.
				 */
				
			Kflag++;
			break;

		case 'k':	/* specify your own stack size */
			if (!ifile) {
				argptr++;
				argcnt--;
			}
			if ((kflag = stoi(*argptr)) == 0x8000) {
				yyerror("-%c flag does not specify a number: %s", flgname[0], *argptr);
				kflag = STKSIZ;
			} else {
				if (kflag < STKSIZ) {
					yyerror("stack size specified less than 8k");
				}
			}
			break;

		case 'R':	/* linking process to take place in Real Address
				 * mode.
				 */
			Rflag++;
			break;
#endif
		default:
			yyerror("unknown flag: %s", flgname);

		}
}
