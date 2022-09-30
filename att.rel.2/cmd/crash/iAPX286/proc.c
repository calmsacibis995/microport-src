/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)proc.c	1.4 - 85/08/12 */
#include	"crash.h"
#include	"sys/proc.h"
#include	"sys/text.h"
extern struct uarea x;

prproc(c, md, run, all)
	int	c;
	int	md;
	int	run;
	int	all;
{
	char	ch, *cp;

	union {
		struct proc	pbuf;
		struct xproc	xpbuf;
	} proc;
	struct proc *p;
	struct xproc *xp;

	int	i;

	if(c == -1)
		return;
	if(c > (int)v.ve_proc) {
		printf("%d out of range, use Process Table Slot\n", c);
		return;
	}
	if(readmem(&proc, (long)SYM_VALUE(Proc) + c * sizeof proc,
		sizeof proc) != sizeof proc) {
			printf("%3d  read error on proc table\n", c);
			return;
	}
	p = &proc.pbuf;
	xp = &proc.xpbuf;
	if(!all && p->p_stat == NULL)
		return;
	if(run && p->p_stat != SRUN)
		return;
	switch(p->p_stat) {
	case 0:      ch = ' '; break;
	case SSLEEP: ch = 's'; break;
	case SWAIT:  ch = 'w'; break;
	case SRUN:   ch = 'r'; break;
	case SIDL:   ch = 'i'; break;
	case SZOMB:  ch = 'z'; break;
	case SSTOP:  ch = 't'; break;
	case SXBRK:  ch = 'x'; break;
	case SXSTK:  ch = 'x'; break;
	default:     ch = '?'; break;
	}
	printf("%3d %c", c, ch);
	printf(" %5u %5u %5u %5u %5u  %2u %3u",
		p->p_pid, p->p_ppid, p->p_pgrp, p->p_uid, p->p_suid,
		p->p_pri & 0377, p->p_cpu & 0377);
#ifdef	pdp11
	p->p_wchan == 0 ? printf("         ") : printf(" %8.8o", p->p_wchan);
#endif
#if	vax 
	p->p_wchan == 0 ? printf("         ") : printf(" %8.8x", p->p_wchan);
#endif
#if iAPX286
	p->p_wchan == 0 ? printf("         ") : printf(" %8.8lx", p->p_wchan);
#endif
	cp = "SWAPPED";
	if(p->p_flag & SLOAD) {
		if(getuarea(c) == -1) {
			cp = "read err on uarea";
		}
		else {
			if(c == 0)
				cp = "swapper";
			else
				cp = x.u.u_comm;
			for(i = 0; i < 8 && cp[i]; i++) {
				if(cp[i] < 040 || cp[i] > 0176) {
					cp = "[unknown]";
					break;
				}
			}
		}
	}
	if(p->p_stat == NULL)
		cp = "";
	else if(p->p_stat == SZOMB)
		cp = "ZOMBIE";
	printf(" %-8s", cp);
	printf("%s%s%s%s%s%s%s%s\n",
		p->p_flag & SLOAD ? " incore" : "",
		p->p_flag & SSYS ? " sched" : "",
		p->p_flag & SLOCK ? " locked" : "",
		p->p_flag & SSWAP ? " swapped" : "",
		p->p_flag & STRC ? " trace" : "",
		p->p_flag & SWTED ? " trace" : "",
		p->p_flag & STEXT ? " text" : "",
		p->p_flag & SSPART ? " part-swap" : "");
	if(md == 0)
		return;
	if(p->p_stat == SZOMB) {
		printf("     exit: %1.1o  user time: %ld  sys time: %ld\n",
		xp->xp_xstat, xp->xp_utime, xp->xp_stime);
		return;
	}
	printf("\tsz:%u sig:%1.1lo tm:%u nice:%d\n\talm:%u",
		p->p_size, p->p_sig, p->p_time, p->p_nice, p->p_clktim);
#if iAPX286
	printf(NBPW==4 ? " q:%3d" : " q:%3d" , ((struct proc *)p->p_link -
		(struct proc *)Proc->n_value));
#else
	printf(NBPW==4 ? " q:%3d" : " q:%3d" , ((unsigned)p->p_link -
		Proc->n_value) / sizeof(struct proc));
#endif
	p->p_textp ?
#if iAPX286
		printf(" tx:%3d\n", ((struct text *)p->p_textp -
						(struct text *)Text->n_value))
#else
		printf(" tx:%3d\n", ((unsigned)p->p_textp - Text->n_value) /
			sizeof(struct text))
#endif
		: printf("\n");
}
