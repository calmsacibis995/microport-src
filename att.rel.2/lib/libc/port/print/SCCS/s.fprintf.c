h08770
s 00002/00002/00037
d D 1.2 87/05/28 10:17:34 root 2 1
c fixed _flag bit check
e
s 00039/00000/00000
d D 1.1 87/05/28 07:10:50 root 1 0
c date and time created 87/05/28 07:10:50 by root
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.4	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <varargs.h>

extern int _doprnt();

/*VARARGS2*/
int
fprintf(iop, format, va_alist)
FILE *iop;
char *format;
va_dcl
{
	register int count;
	va_list ap;

	va_start(ap);
D 2
	if (!(iop->_flag | _IOWRT)) {
E 2
I 2
	if (!(iop->_flag & _IOWRT)) {
E 2
		/* if no write flag */
D 2
		if (iop->_flag | _IORW) {
E 2
I 2
		if (iop->_flag & _IORW) {
E 2
			/* if ok, cause read-write */
			iop->_flag |= _IOWRT;
		} else {
			/* else error */
			return EOF;
		}
	}
	count = _doprnt(format, ap, iop);
	va_end(ap);
	return(ferror(iop)? EOF: count);
}
E 1
