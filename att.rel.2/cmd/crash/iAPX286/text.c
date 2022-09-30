/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)text.c	1.3 - 85/08/09 */
#include	"crash.h"
#include	"sys/text.h"
#include	"sys/inode.h"
#include	"sys/proc.h"

prtext(c, all)
	int	c;
	int	all;
{
	struct	text	tbuf;
	char	x_flag;

	if(c == -1)
		return;
	if(c >= v.v_text) {
		printf("%4d  out of range\n", c);
		return;
	}
	if(readmem(&tbuf, (long)SYM_VALUE(Text) + c * sizeof tbuf,
		sizeof tbuf) != sizeof tbuf) {
			printf("%4d  read error on text table\n", c);
			return;
	}
	if(!all && tbuf.x_iptr == NULL)
		return;
	printf("%4u  %5u  %3u  %5u",
		c, tbuf.x_iptr ?
#if iAPX286
		(tbuf.x_iptr - (struct inode *)Inode->n_value):0,
#else
		((unsigned)tbuf.x_iptr-Inode->n_value)/sizeof(struct inode):0,
#endif
		tbuf.x_count, tbuf.x_ccount);
#if iAPX286
	printf("  %04x",tbuf.x_caddr);
#endif
#ifdef	vax
	printf("  %4u",
		tbuf.x_caddr ?
		((unsigned)tbuf.x_caddr-Proc->n_value)/sizeof(struct proc):0);
#endif
#ifdef	pdp11
	printf("  %7o", tbuf.x_caddr);
#endif
	printf("  %7u   %4u  ",
		tbuf.x_daddr, tbuf.x_size);
	x_flag = tbuf.x_flag;
	printf("%s%s%s%s%s\n",
		x_flag & XTRC ? "exclu-write" : "",
		x_flag & XWRIT ? "write" : "",
		x_flag & XLOAD ? "load" : "",
		x_flag & XLOCK ? "lock" : "",
		x_flag & XWANT ? "want" : "");
}
