/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/*
 *	prfpr - print profiler log files
 */

#include "stdio.h"
#include "time.h"
#include "filehdr.h"
#include "scnhdr.h"
#include "syms.h"
#include "ldfcn.h"

#define PRFMAX  2048		/* max number of text symbols */
#define LDNULL	(LDFILE *)0	/* a null that libld likes	*/

struct	profile	{
	long	p_date;		/* time stamp of record */
	long	p_ctr[PRFMAX+2];	/* profiler counter values */
} p[2];

int	prfmax;			/* actual number of text symbols */

long	t[PRFMAX+2];
SYMENT	*sym;
int	symcnt;

char	*namelist = "/unix";
char	*logfile;
LDFILE *ldptr;			/* file pointer for libld	*/
int	log;

long	sum, osum;


double	pc;
double	cutoff = 1e-2;
main(argc, argv)
char **argv;
{
	int ff, i;
	double	atof();
	SYMENT	*search(), *sp;
	char	*ldgetname();

	switch(argc) {
		case 4:
			namelist = argv[3];
		case 3:
			cutoff = atof(argv[2]) / 1e2;
		case 2:
			logfile = argv[1];
			break;
		default:
			error("usage: prfpr file [ cutoff [ namelist ] ]");
	}
	if((log = open(logfile, 0)) < 0)
		error("cannot open data file");
	if(cutoff >= 1e0 || cutoff < 0e0)
		error("invalid cutoff percentage");
	if(read(log, &prfmax, sizeof prfmax) != sizeof prfmax || prfmax == 0)
		error("bad data file");
	if(read(log, t, prfmax * sizeof (long)) != prfmax * sizeof (long))
		error("cannot read profile addresses");

	osum = sum = ff = 0;

	read(log, &p[!ff], (prfmax + 2) * sizeof (long));
	for(i = 0; i <= prfmax; i++)
		osum += p[!ff].p_ctr[i];

	rdsymtab();
	for(;;) {
		sum = 0;
		if(read(log, &p[ff], (prfmax + 2) * sizeof (long)) !=
		    (prfmax + 2) * sizeof (long))
			exit(0);
		shtime(&p[!ff].p_date);
		shtime(&p[ff].p_date);
		printf("\n");
		for(i = 0; i <= prfmax; i++)
			sum += p[ff].p_ctr[i];
		if(sum == osum)
			printf("no samples\n\n");
		else for(i = 0; i <= prfmax; i++) {
			pc = (double) (p[ff].p_ctr[i] - p[!ff].p_ctr[i]) /
				(double) (sum - osum);
			if(pc > cutoff)
				if(i == prfmax)
					printf("user     %5.2f\n",
					 pc * 1e2);
				else {
					sp = search(t[i]);
					if(sp == 0)
						printf("unknown  %5.2f\n",
							pc * 1e2);
					else
						printf("%-8.8s %5.2f\n",
					 	ldgetname(ldptr, sp), pc * 1e2);
				}
		}
		ff = !ff;
		osum = sum;
		printf("\n");
	}
}

error(s)
char *s;
{
	printf("error: %s\n", s);
	exit(1);
}


SYMENT *
search(addr)
unsigned long addr;
{
	SYMENT		*sp, *save;
	unsigned long	value;

	value = 0;
	save = 0;
	for(sp = sym; sp < &sym[symcnt]; sp++) {
		if( sp->n_value <= addr && sp->n_value > value) {
			value = sp->n_value;
			save = sp;
		}
	}
	return(save);
}

rdsymtab()
{
	SYMENT		sp;
	int		n_text;
	unsigned long	j;

	if((ldptr = ldopen(namelist, LDNULL)) == LDNULL) {
		perror(namelist);
		exit(1);
	}
	if (( TYPE( ldptr ) != I286SMAGIC )
	    && ( TYPE( ldptr ) != I286LMAGIC )) {
		error("not a valid iAPX286 file");
	}

	n_text = findmax();

	if(ldtbseek(ldptr) == FAILURE) {
		perror("ldtbseek");
		exit(1);
	}

	if ((sym = (SYMENT *)malloc(HEADER(ldptr).f_nsyms * SYMESZ)) == NULL)
		error("can't allocate memory for symbols");

	for (j = 0; j < HEADER(ldptr).f_nsyms; j++) {
		if (FREAD((char *)&sp, SYMESZ, 1, ldptr) == FAILURE) {
			perror("FREAD");
			exit(1);
		}
		if(sp.n_sclass == C_EXT && sp.n_scnum <= n_text) {
			if (symcnt == PRFMAX)
				error("too many text symbols");
			sym[ symcnt++ ] = sp;
		}
	}
}

/*
 * findmax
 *	read in the section headers of 'namelist'
 *	and return the largest section number 
 *	associated with .text
 */
findmax()
{
	SCNHDR	scnhdr;		/* place to read in section header	*/
	int	i;
	int	maxsect = 1;	/* maximum section number		*/

	for ( i = 1; i <= HEADER( ldptr ).f_nscns; i++ )
	{
		if ( ldshread( ldptr, i, &scnhdr ) == FAILURE )
		{
			perror( "ldshread" );
			exit( 1 );
		}

		if ( strcmp( scnhdr.s_name, ".text" ) == 0 )
		{
			maxsect = i;
		}
	}

	return( maxsect );
}

shtime(l)
register long *l;
{
	register  struct  tm  *t;
	struct  tm  *localtime();

	if(*l == (long) 0) {
		printf("initialization\n");
		return;
	}
	t = localtime(l);
	printf("%02.2d/%02.2d/%02.2d %02.2d:%02.2d\n", t->tm_mon + 1,
		t->tm_mday, t->tm_year, t->tm_hour, t->tm_min);
}
