/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)spl.c	1.5 */
#include	"../sys/8259.h"

/*
** tables of masks that splx uses to determine proper mask for level
*/
char	mastbl[] =	/* master PIC table			*/
{
	SPL0MASTER,	/* spl0 - enable all			*/
	SPL7MASTER,	/* spl1 - unused level			*/
	SPL7MASTER,	/* spl2 - unused level			*/
	SPL7MASTER,	/* spl3 - unused level			*/
	SPL4MASTER,	/* spl4 - disable console		*/
	SPL5MASTER,	/* spl5 - disable cons and 544		*/
	SPL6MASTER,	/* spl6 - disable cons,544,215		*/
	SPL7MASTER	/* spl7 - disable cons,544,215,clock	*/
};

char	slavtbl[] =	/* slave PIC table			*/
{
	SPL7SLAVE,	/* spl0 - unused level			*/
	SPL7SLAVE,	/* spl1 - unused level			*/
	SPL7SLAVE,	/* spl2 - unused level			*/
	SPL7SLAVE,	/* spl3 - unused level			*/
	SPL7SLAVE,	/* spl4 - unused level			*/
	SPL7SLAVE,	/* spl5 - unused level			*/
	SPL7SLAVE,	/* spl6 - unused level			*/
	SPL7SLAVE	/* spl7 - unused level			*/
};
