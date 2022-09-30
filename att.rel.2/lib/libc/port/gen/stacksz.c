/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/* Find the stack space in bytes required to run a program */
/* C program should be compiled with -p flag */
/* spmax and spmin are used by monitor */
unsigned spmax,spmin;
void _stacksz(ii) unsigned ii;
{
	if(!spmin)  /* first entry,ie from mcrt0 */
		spmax = spmin = ii;
	else if(spmax >ii) spmax = ii;
}
