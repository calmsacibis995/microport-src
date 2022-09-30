/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)format.c	1.6 */
static char sccsid[] = "@(#)format.c	1.6" ;
#include <stdio.h>
#include <sys/types.h>
#include <sys/wn.h>
#include <fcntl.h>


unsigned long first = 0 ;
unsigned long last = -1 ;
unsigned int interleave = 3 ;
char *device = NULL ;

main(argc,argv)
char **argv ;
{
	int i ;

	for(i=1;i<argc;i++)
	if(argv[i][0] == '-')
		switch( argv[i][1] ) {

		case 'f':
			if( i++ <argc )
				first = atol( argv[i] ) ;
			else
				giveusage() ;
			break ;

		case 'l':
			if( i++ <argc )
				last = atol( argv[i] ) ;
			else
				giveusage() ;
			break ;

		case 'i':
			if( i++ <argc )
				interleave = atoi( argv[i] ) ;
			else
				giveusage() ;
			break ;

		default:
			giveusage() ;
			break ;

		}
	else
	{
		if ( device == NULL )
			device = argv[i] ;
		else
			giveusage() ;
	}

	if ( device == NULL )
		giveusage() ;

	doformat() ;

}

giveusage()
{
	fprintf(stderr,"Usage: format [-f first] [-l last] [-i interleave] device\n");
	exit() ;
}

int dev ;

doformat()
{
	long i ;
	struct i215ftk control ;

#if DEBUG
fprintf(stderr,"doformat -f %ld -l %ld -i %d %s\n",first,last,interleave,device);
#endif

	if( (dev=open(device,O_RDWR)) == -1)
	{
		perror("format: opening device");
		exit() ;
	}

	if ( last == -1)
	{
		/*
		 *	last wasn't specified - get it for them
		 */
		
		if( ioctl( dev , I215_NTRACK , &last ) == -1 )
		{
			perror("format: performing ioctl on device");
			exit();
		}
#if DEBUG
fprintf(stderr,"last (as returned by ioctl) %ld (0x%lx)\n",last,last);
#endif
		last-- ;
	}

	/*
	 *	check for absurd
	 */

	if( first > last)
	{
		fprintf(stderr,"format: first > last\n");
		exit();
	}

	control.f_intl = interleave ;
	control.f_skew = 0 ;		/* ignored */
	control.f_type = FORMAT_DATA ;
	control.f_pat[0] = 'D' ;
	control.f_pat[1] = 'e' ;
	control.f_pat[2] = 'a' ;
	control.f_pat[3] = 'D' ;


	for(i=first;i<=last;i++)
	{
		control.f_track = i ;
		if( ioctl(dev,I215_IOC_FMT,&control) == -1)
		{
			perror("format: performing ioctl");
			break  ;
		}
	}

	if((i!=0)&&(first <= (i-1)) )
		printf("Formatted tracks %ld to %ld of device %s\n",first,i-1,device);

	close(dev);
}
