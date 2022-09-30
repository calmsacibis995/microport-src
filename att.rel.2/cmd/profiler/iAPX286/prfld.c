/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/*
 *	prfld - load profiler with sorted kernel text addresses
 */

#include "stdio.h"
#include "errno.h"
#include "filehdr.h"
#include "scnhdr.h"
#include "syms.h"
#include "ldfcn.h"

#define PRFMAX	2048		/* maximum number of symbols */
#define LDNULL	(LDFILE *)0	/* a null that libld likes	*/

char *namelist = "/unix";	/* namelist file */
extern	int	errno;
LDFILE *ldptr;			/* file pointer for libld	*/

main(argc, argv)
char **argv;
{
	register long *ip; 
	int		prf;
	SYMENT	sp;
	long taddr[PRFMAX];
	int ntaddr;
	int	compar();
	short n_text;			/* largest section number of .text */

	if(argc == 2)
		namelist = argv[1];
	else if(argc != 1)
		error("usage: prfld [/unix]");
	if((prf = open("/dev/prf", 1)) < 0)
		error("cannot open /dev/prf");
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

	ip = taddr;
	*ip++ = 0;
	while (FREAD((char *)&sp, sizeof(sp), 1, ldptr) != FAILURE) {
		if(ip == &taddr[PRFMAX])
			error("too many text symbols");
		if(sp.n_sclass == C_EXT && sp.n_scnum <= n_text)
			*ip++ = sp.n_value;
	}
	ntaddr = ip - taddr;
	qsort((char *)taddr, ntaddr, sizeof (long), compar);
	if(write(prf, taddr, ntaddr*sizeof(long)) != ntaddr*sizeof(long))
		switch(errno) {
		case ENOSPC:
			error("insufficient space in system for addresses");
		case E2BIG:
			error("unaligned data or insufficient addresses");
		case EBUSY:
			error("profiler is enabled");
		case EINVAL:
			error("text addresses not sorted properly");
		default:
			error("cannot load profiler addresses");
		}
}

compar(x, y)
	register  unsigned  long *x, *y;
{
	if(*x > *y)
		return(1);
	else if(*x == *y)
		return(0);
	return(-1);
}

error(s)
char *s;
{
	printf("error: %s\n", s);
	exit(1);
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
