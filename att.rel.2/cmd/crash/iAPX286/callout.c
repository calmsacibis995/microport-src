/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)callout.c	1.3 - 85/08/09 */
#include	"crash.h"
#include	"sys/callo.h"
#include	"sys/map.h"

#define	NMAP	50
#define	NCALL	50
#if iAPX286
struct syment * nmsrch(),* search();
long sysvad();
long lseek();
#endif

prcallout()
{
	register  int  i;
	register  struct  syment  *sp;
	struct  callo  callout[NCALL];

#if iAPX286
	long physaddr;

	if((physaddr=sysvad((long)SYM_VALUE(Callout),sizeof (struct callo)))==-1L)
	{
		error("callout structure address incorrect");
		return;
	}
	if(lseek(mem,physaddr,0)==-1L)
	{
		error("seek for callout structure failed");
		return;
	}
#else
	lseek(mem, (long)SYM_VALUE(Callout), 0);
#endif
	for(;;) {
		if(read(mem, callout, sizeof callout) != sizeof callout) {
			error("read error on callout structure");
			return;
		}
		for(i = 0; i < NCALL; i++) {
			if(callout[i].c_func == NULL)
				return;
			sp = (struct syment *)search(callout[i].c_func,
#if vax || iAPX286
				N_TEXT, N_TEXT);
#else
				(N_TEXT | N_EXT), (N_TEXT | N_EXT));
#endif
			if(sp == NULL)
				printf("no match\n");
			else {
				(sp->n_name[0] == '_') ?
					printf("%-7.7s  ", &sp->n_name[1]) :
#if iAPX286
					printf("%-8.8s ", sp->n_name);
#else
					printf("%-8.8s  ", &sp->n_name[1]);
#endif
#ifdef	pdp11
				printf("%8.6o", callout[i].c_arg);
#endif
#ifdef	vax
				printf("%8.8x", callout[i].c_arg);
#endif
#if iAPX286
				printf("%8.8lx", callout[i].c_arg);
#endif
				printf(" %5u\n",callout[i].c_time);
			}
		}
	}
}

prmap(s)
	register  char  *s;
{
	struct	map	mbuf[NMAP];
	register  int  i;
	unsigned  free = 0, seg = 0;
	register  struct  syment  *sp;
#if iAPX286
	long physaddr;
#endif

	printf("%s\n", s);
	if((sp = (struct syment *)nmsrch(s)) == NULL) {
		printf("symbol not found\n");
		return;
	}
	printf("address  size\n");
#if iAPX286
	if((physaddr=sysvad((long)SYM_VALUE(sp),sizeof mbuf))==-1L)
	{
		error("map structure address incorrect");
		return;
	}
	if(lseek(mem,physaddr,0)==-1L)
	{
		error("seek for map structure failed");
		return;
	}
#else
	lseek(mem, SYM_VALUE(sp), 0);
#endif
	for(;;) {
		if(read(mem, mbuf, sizeof mbuf) != sizeof mbuf) {
			error("read error on map structure");
			return;
		}
		for(i = 0; i < NMAP; i++) {
			if(mbuf[i].m_size == 0) {
				printf("%u segments, %u units\n", seg, free);
				return;
			}
#if iAPX286
			printf(" %.4x  ", mbuf[i].m_addr);
#else
			printf(FMT, mbuf[i].m_addr);
#endif
			printf(" %5u\n", mbuf[i].m_size);
			free += mbuf[i].m_size;
			seg++;
		}
	}
}
