/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.3	*/
/*LINTLIBRARY*/
/*
 *	Environment variable PROFDIR added such that:
 *		If PROFDIR doesn't exist, "mon.out" is produced as before.
 *		If PROFDIR = NULL, no profiling output is produced.
 *		If PROFDIR = string, "string/pid.progname" is produced,
 *		  where name consists of argv[0] suitably massaged.
 */
#include <mon.h>
#include <sys/mmu.h>
#ifdef iAPX286
#ifdef SMALL_M
#define	ADDR	(((long)((CODE1_SEL) << 3 | LDT_TI | USER_RPL) << 16))
#endif
#define ECORRECT 2
#endif
#define PROFDIR	"PROFDIR"

extern int creat(), write(), close(), getpid();
extern void profil(), perror();
extern char *getenv(), *strcpy(), *strrchr();
#ifdef iAPX286
unsigned spmax,spmin; /* stack size, set up by mcount() */
#endif

char **___Argv = NULL; /* initialized to argv array by mcrt0 (if loaded) */
char *_countbase;

static int ssiz;
static struct hdr *sbuf;
static char mon_out[100];

void
monitor(lowpc, highpc, buffer, bufsize, nfunc)
char	*lowpc, *highpc;	/* boundaries of text to be monitored */
WORD	*buffer;	/* ptr to space for monitor data (WORDs) */
int	bufsize;	/* size of above space (in WORDs) */
int	nfunc;		/* max no. of functions whose calls are counted
			(default nfunc is 300 on PDP11, 600 on others) */
{
	int scale;
	long text;
	register char *s, *end, *name = mon_out;
#if iAPX286
	long scalel;
#endif

	if (lowpc == NULL) {		/* true only at the end */
		if (sbuf != NULL) {
			int fd;

			profil((char *)NULL, 0, 0, 0);
#ifdef iAPX286
			sbuf->stack  = spmin - spmax;		
			sbuf->stacke = spmax + ECORRECT;
#endif
			if ((fd = creat(mon_out, 0666)) < 0 ||
			    write(fd, (char *)sbuf, (unsigned)ssiz) != ssiz)
				perror(mon_out);
			if (fd >= 0)
				(void) close(fd);
		}
		return;
	}
	_countbase = (char *)buffer + sizeof(struct hdr);
	sbuf = NULL;
	ssiz = (sizeof(struct hdr) + nfunc * sizeof(struct cnt))/sizeof(WORD);
	if (ssiz >= bufsize || lowpc >= highpc)
		return;
	if ((s = getenv(PROFDIR)) == NULL) /* PROFDIR not in environment */
		end = MON_OUT; /* use default "mon.out" */
	else if (*s == '\0') /* value of PROFDIR is NULL */
		return; /* no profiling on this run */
	else { /* construct "PROFDIR/pid.progname" */
		register int pid, n;

		while (*s != '\0') /* copy PROFDIR value (path-prefix) */
			*name++ = *s++;
		*name++ = '/'; /* two slashes won't hurt */
		if ((pid = getpid()) <= 0) /* extra test just in case */
			pid = 1; /* getpid returns something inappropriate */
		for (n = 10000; n > pid; n /= 10)
			; /* suppress leading zeros */
		for ( ; ; n /= 10) {
			*name++ = pid/n + '0';
			if (n == 1)
				break;
			pid %= n;
		}
		*name++ = '.';
		if (___Argv != NULL) /* mcrt0.s executed */
			end = ((s = strrchr(___Argv[0], '/')) != NULL) ?
			    s + 1 : /* use only file-name part */
			    ___Argv[0]; /* use entire name */
	}
	(void) strcpy(name, end); /* copy final element of file-name */
	sbuf = (struct hdr *)buffer;	/* for writing buffer at the wrapup */
#ifdef iAPX286
	sbuf->lpc = (unsigned long)lowpc;	/* initialize the first */
	sbuf->hpc = (unsigned long)highpc;	/* region of the buffer */
#ifdef SMALL_M
	sbuf->lpc |= ADDR; /* force addresses to be 32 bit */
	sbuf->hpc |= ADDR;
#endif
#else
	sbuf->lpc = lowpc;	/* initialize the first */
	sbuf->hpc = highpc;	/* region of the buffer */
#endif
	sbuf->nfns = nfunc;
	buffer += ssiz;			/* move ptr past 2'nd region */
	bufsize -= ssiz;		/* no. WORDs in third region */
					/* no. WORDs of text */
#ifdef iAPX286
#ifdef SMALL_M
	text = (highpc - lowpc + sizeof(WORD) - 1)/
			sizeof(WORD);
#else
	/* shift seg address right 3 to get text size */
	text = ( INDEX(highpc) - INDEX(lowpc) + sizeof(WORD) - 1)/
			sizeof(WORD);
#endif
#else
	text = (highpc - lowpc + sizeof(WORD) - 1)/
			sizeof(WORD);
#endif
	/* scale is a 16 bit fixed point fraction with the decimal
	   point at the left */
	if (bufsize < text)  {
		/* make sure cast is done first! */
		double temp = (double)bufsize;
#if iAPX286
		scalel = (temp * (long)0200000L) / text;  
		scale = scalel ;	/* avoid int overflow */
#else
		scale = (temp * (long)0200000L) / text;  
#endif
	} else  {
		/* scale must be less than 1 */
		scale = 0xffff;
	}
	bufsize *= sizeof(WORD);	/* bufsize into # bytes */
	ssiz = ssiz * sizeof(WORD) + bufsize;	/* size into # bytes */
#ifdef iAPX286
	profil((char *)buffer, bufsize, ( void(*)())lowpc, scale);
#else
	profil((char *)buffer, bufsize, (int)lowpc, scale);
#endif
}
