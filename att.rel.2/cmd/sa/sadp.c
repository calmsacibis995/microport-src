/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*
** iAPX286 disk profiler
** usage : sadp [-th][-d device[-ctlrnumber]] s [n]
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/sysmacros.h>
#include	<sys/buf.h>
#include	<sys/elog.h>
#include	<nlist.h>
#include	<sys/iobuf.h>
#include	<time.h>
#include	<sys/utsname.h>
#include	<sys/var.h>
#include	<ctype.h>
#include	<string.h>

#define	OSFILE	"/unix"
#define	MEMFILE	"/dev/kmem"
#define	MAXCYL	512		/* maximum cylinder number		*/
#define	NDRIVE	4		/* number of drives per controller	*/
#define	GRAN	8		/* granularity of profile		*/
#define	LINELEN	128		/* length of a print line		*/
#define	BLOT	'*'
#define	HSLOP	4	/* # of chars on histogram before actual data	*/
#define NUMCHK	( MAXCYL / GRAN )	/* number of GRAN chunks of cyls*/
#define	DIVSIZ	( LINELEN / NUMCHK )	/* division size for histogram  */
#define	CYL	1
#define	SEEK	2
#define	FFEED	'\014'

/*
** offsets into the setup array
*/
#define	WNS	0
#define	WNHEAD	1
#define	WNCNT	2

struct nlist setup[] =
{
	{ "wnstat" },		/* disk statistics		*/
	{ "i215tab" },		/* driver request queue		*/
	{ "wn_cnt" },		/* count of devices present	*/
	{ "" }
};
 
struct buf	curbuf;		/* current buffer header being examined	*/
struct buf	*bp;		/* pointer into mem for current buffer	*/
struct iobuf	*wnqhead;	/* head of drivers queue		*/
struct iotime	*b4wnstats;	/* disk stats before profiling		*/
struct iotime	*afwnstats;	/* disk stats after profiling		*/
struct utsname	name;
struct hstruct
{
	unsigned long	count;
	unsigned int	startnum;
} hist[ NUMCHK ];

char		pline[ LINELEN + 1];	/* print line (+1 for null)	*/
unsigned long	*cylinder;
unsigned long	*distance;
unsigned int	prevcyl;
 
#define	NUMCHARS	3
char devnm[ NDRIVE ][ NUMCHARS ] =
{
	"wn",
	"wn",
	"wn",
	"wn"
};
 
int		fflg, dflg, tflg, hflg, errflg;
int		seconds, repeat;
char		*device;
int		memfd;
int		wncnt;
unsigned long	iosampled;
unsigned long	ioseeked;
int		controller;
 
char	*malloc();
char	*calloc();
long	lseek();
char	*ctime();
long	time();
int	compar();

 
main( argc, argv )
int	argc;
char	**argv;
{
	extern int	optind;
	extern char	*optarg;
	int		c;
	int		i;
	int		j;
	unsigned long	iocnt;
	unsigned long	tmpcnt;
	long		date;
	int		dev;
	int		numctl;
	struct iobuf	*qaddr;
	struct iotime	*stataddr;

	while ( ( c = getopt( argc, argv, "thd:" ) ) != EOF )
	{
		switch( c )
		{
			case 't':
				tflg++;
				break;

			case 'h':
				hflg++;
				break;

			case 'd':
				device = optarg;
				/*
				** find out if this is a recognized name
				*/
				for ( i = 0; i < NDRIVE; i++ )
				{
					if ( strncmp( device, devnm[ i ], NUMCHARS - 1 ) == 0 )
					{
						break;
					}
				}

				if ( i == NDRIVE )
				{
					fprintf( stderr,
						"sadp: device %s not known\n",
						 device );
					fprintf( stderr,
						"recognized device is %s\n",
						devnm[ 0 ] );
					++errflg;
					break;
				}
				
				/*
				** see if there is a -device spec
				*/
				optarg += ( NUMCHARS - 1 );
				if ( *optarg == '-' )
				{
					++optarg;
					if ( tstdigit( optarg ) )
					{
						controller = atoi( optarg );
					}
					else
					{
						fprintf( stderr,
						"sadp: %s is not numeric\n",
							optarg );
						usage();
					}
				}

				dflg++;
				break;

			case '?':
				errflg++;
				break;

		}
	}

	if ( errflg )
		usage();

	/*
	** if no frequency arguments present, exit
	*/
	if ( optind == argc )
	{
		fprintf( stderr, "sadp: frequency not specified\n" );
		usage();
	}

	/*
	** check if seconds are numeric
	*/
	if ( ! tstdigit( argv[ optind ] ) )
	{
		fprintf( stderr, "sadp: %s is not numeric\n", argv[ optind ] );
		usage();
	}

	/*
	** for frequency arguments, if only seconds is present,
	** set repeat to 1
	*/
	if ( ( optind + 1 ) == argc )
	{
		seconds = atoi( argv[ optind ] );
		repeat = 1;
	}

	/*
	** if both seconds and repeat are specified, check if 
	** repeat is numeric.
	*/
	if ( ( optind + 1 ) < argc )
	{
		if ( ! tstdigit( argv[ optind + 1 ] ) )
		{
			fprintf( stderr, "sadp: %s is not numeric\n",
				 argv[ optind + 1 ] );
			usage();
		}
		seconds = atoi( argv[ optind ] );
		repeat = atoi( argv[ optind + 1 ] );
	}
	if ( seconds <= 0 )
	{
		extmsg( "seconds must be > 0" );
	}
	if ( repeat <= 0 )
	{
		extmsg( "repeat count must be > 0" );
	}

	if ( nlist( OSFILE, setup ) == -1 )
		extmsg( "can't get namelist" );

	if ( ( memfd = open( MEMFILE, 0 ) ) == -1 )
	{
		fprintf( stderr, "sadp: " );
		perror( MEMFILE );
		exit( 1 );
	}

	if ( setup[ WNCNT ].n_value != NULL )
	{
		lseek( memfd, (long)setup[ WNCNT ].n_value, 0 );
		if ( read( memfd, &wncnt, sizeof wncnt ) == -1 )
			extmsg( "cannot read disk counter" );
	}
	else
		extmsg( "no disk counter" );

	/*
	** calculate how many controllers there are.
	*/
	numctl = ( wncnt / ( NDRIVE + 1 ) ) + 1;

	/*
	** allocate memory for iostat structures, and wnqheads
	*/
	if ( ( wnqhead =
		(struct iobuf *)calloc( numctl, sizeof( struct iobuf ) ) ) == NULL )
	{
		extmsg( "can't get memory for driver queue heads" );
	}
	if ( ( b4wnstats =
		(struct iotime *)calloc( numctl, sizeof( struct iotime ) ) ) == NULL )
	{
		extmsg( "can't get memory for driver statistics" );
	}
	if ( ( afwnstats =
		(struct iotime *)calloc( numctl, sizeof( struct iotime ) ) ) == NULL )
	{
		extmsg( "can't get memory for driver statistics" );
	}

	/*
	** attempt to allocate ram for cylinder counter array, and for
	** seek distance array
	*/
	if ( ( cylinder =
		(unsigned long *)calloc( MAXCYL, sizeof( long ) ) ) == NULL )
	{
		extmsg( "can't get memory for cylinder counters" );
	}
	if ( ( distance =
		(unsigned long *)calloc( MAXCYL, sizeof( long ) ) ) == NULL )
	{
		extmsg( "can't get memory for distance counters" );
	}

	/*
	** now, the disk driver queue head is read in, and its forward
	** link is used to get to the first request. From then on, the
	** disk queue is traversed, remembering the cylinder number
	** of the request, and comparing the current cylinder number
	** with the previous number to get seek distance
	*/
	if ( setup[ WNHEAD ].n_value == NULL )
	{
		extmsg( "can't find head of disk queue" );
	}

	if ( setup[ WNS ].n_value == NULL )
	{
		extmsg( "can't find disk statistics" );
	}

	/*
	** do this next stuff until seconds expires, and repeat count
	** is exhausted
	*/
	while( repeat-- )
	{
	/*
	** if a controller was specified, use it. otherwise, do as many
	** controllers that are specified by wncnt
	*/
	if ( controller )
	{
		dev = controller;
		goto doit;
	}
	for ( dev = 0; dev < numctl; dev++ )
	{
doit:
		/*
		** calculate address of head of queue
		*/
		qaddr = (struct iobuf *)setup[ WNHEAD ].n_value +
			( dev * sizeof( struct iobuf ) );

		/*
		** calculate address of iostat struct
		*/
		stataddr = (struct iotime *)setup[ WNS ].n_value +
			( dev * sizeof( struct iotime ) );

		/*
		** get the drive io statistics before starting
		*/
		lseek( memfd, (long)stataddr, 0 );
		if ( read( memfd, &b4wnstats[ dev ], sizeof( struct iotime ) ) == -1 )
		{
			extmsg( "can't read in disk statistics" );
		}

		prevcyl = 0;
		i = seconds;
		while ( i-- )
		{
			lseek( memfd, (long)qaddr, 0 );
			if ( read( memfd, &wnqhead[ dev ], sizeof( struct iobuf ) ) == -1 )
			{
				extmsg( "can't read in head of disk queue" );
			}

			/*
			** point to first buf header
			*/
			bp = wnqhead[ dev ].b_actf;

			/*
			** while there are more buffers to look at
			*/
			while ( bp != (struct buf *)qaddr && ( bp != NULL ) )
			{
				lseek( memfd, (long)bp, 0 );
				if ( read( memfd, &curbuf, sizeof( struct buf ) ) == -1 )
				{
					fprintf( stderr,
						"can't read buffer @ 0x%lx\n",
						 bp );
					exit( 1 );
				}

				/*
				** if we have found a buffer that is done,
				** we are not in the drivers queue anymore
				*/
				if ( curbuf.b_flags & B_DONE )
					break;

				/*
				** increment cylinder count, and distance
				** between cylinder count
				*/
				++cylinder[ curbuf.b_cylin ];
				++distance[ abs( curbuf.b_cylin - prevcyl ) ];

				prevcyl = curbuf.b_cylin;

				/*
				** point to the next buffer header
				*/
				bp = curbuf.av_forw;
			}

			/*
			** wait a second, then look again
			*/
			sleep( 1 );
		}

		/*
		** get the drive io statistics again
		*/
		lseek( memfd, (long)stataddr, 0 );
		if ( read( memfd, &afwnstats[ dev ], sizeof( struct iotime ) ) == -1 )
		{
			extmsg( "can't read in disk statistics" );
		}

		/*
		** find out how many requests really happened
		*/
		iocnt = afwnstats[ dev ].io_cnt - b4wnstats[ dev ].io_cnt;

		/*
		** get and print out the date
		*/
		time( &date );
		putchar( FFEED );
		printf( "\n\n%s\n", ctime( &date ) );

		/*
		** get system name and print it
		*/
		uname( &name );
		printf( "%s %s %s %s %s\n",
			 name.sysname,
			 name.nodename,
			 name.release,
			 name.version,
			 name.machine );

		/*
		** calculate total io sampled and total seeks
		*/
		iosampled = ioseeked = 0L;
		for ( i = 0; i < MAXCYL; i++ )
		{
			iosampled += cylinder[ i ];
			ioseeked += distance[ i ];
		}

		/*
		** print the reports
		*/
		if ( tflg || ! hflg )
		{
			printf( "\nCYLINDER ACCESS PROFILE\n" );
			printf( "\n%s-%d:\n", devnm[ dev ], dev );
			printf( "Cylinders\tTransfers\n" );

			for ( i = 0; i < MAXCYL; i += GRAN )
			{
				tmpcnt = 0;
				for ( j = 0; j < GRAN; j++ )
				{
					tmpcnt += cylinder[ i + j ];
				}
				if ( tmpcnt > 0L )
				{
					printf( "%3d - %3d\t%ld\n",
						 i, i + ( GRAN - 1 ), tmpcnt );
				}
			}

			printf( "\nSampled I/O = %ld, Actual I/O = %ld\n",
				   iosampled, iocnt );

			if ( iocnt > 0L )
			{
				printf( "Percentage of I/O sampled = %2.2f\n",
				 ( (float)iosampled / (float)iocnt ) * 100.0 );
			}

			printf( "\n\nSEEK DISTANCE PROFILE\n" );
			printf( "\n%s-%d:\n", devnm[ dev ], dev );
			printf( "Seek Distance\tSeeks\n" );

			for ( i = 0; i < MAXCYL; i += GRAN )
			{
				tmpcnt = 0;
				for ( j = 0; j < GRAN; j++ )
				{
					tmpcnt += distance[ i + j ];
				}
				if ( tmpcnt > 0L )
				{
					printf( "%3d - %3d\t%ld\n",
						 i, i + ( GRAN - 1 ), tmpcnt );
				}
			}
			printf( "\nTotal Seeks = %ld\n", ioseeked );
		}

		if ( hflg )
		{
			histogram( cylinder, CYL, dev );
			histogram( distance, SEEK, dev );
			/*
			** clear out the hist array
			*/
			for ( i = 0; i < NUMCHK; i++ )
			{
				hist[ i ].count = 0L;
				hist[ i ].startnum = 0;
			}
		}

		/*
		** now, clear out the count arrays
		*/
		for ( i = 0; i < MAXCYL; i++ )
		{
			cylinder[ i ] = distance[ i ] = 0L;
		}
	}
	}
}

usage()
{
	fprintf(stderr,"usage:  sadp [-th][-d device[-ctlrnumber]] s [n]\n");
	exit(1);
}

extmsg(msg)
char	*msg;
{
	fprintf(stderr, "sadp: %s\n", msg);
	exit(4);
}

tstdigit( ss )
char	*ss;
{
	int	kk,
		cc;
	kk = 0;
	while ( ( cc = ss[ kk ] ) != '\0' )
	{
		if ( isdigit( cc ) == 0 )
			return( 0 );
		kk++;
	}
	return( 1 );
}

histogram( array, type, dev )
unsigned long	*array;
int		type;
int		dev;
{
	int		i;
	int		j;
	unsigned long	tmpcnt;
	unsigned long	maxcnt;
	int		startindex;
	char		tmpstr[ HSLOP + 1 ];
	int		once;
	unsigned long	total;

	/*
	** first, fill up the hist array with the starting
	** number of GRAN number of things, and the counts of
	** these things
	*/
	for ( i = 0; i < MAXCYL; i += GRAN )
	{
		tmpcnt = 0L;
		for ( j = 0; j < GRAN; j++ )
		{
			tmpcnt += array[ i + j ];
		}
		if ( tmpcnt > 0L )
		{
			hist[ i / GRAN ].startnum = i;
			hist[ i / GRAN ].count = tmpcnt;
		}
	}

	/*
	** now, sort the hist array to be in
	** count order ( highest count first )
	*/
	qsort( (char *)hist, NUMCHK, sizeof( struct hstruct ), compar );

	if ( type == CYL )
	{
		total = iosampled;
		putchar( FFEED );
		printf( "\nCYLINDER ACCESS HISTOGRAM\n\n" );
		printf( "%s-%d:\n", devnm[ dev ], dev );
		printf( "Total transfers = %ld\n\n", total );
	}
	else
	{
		total = ioseeked;
		putchar( FFEED );
		printf( "\nSEEK DISTANCE HISTOGRAM\n\n" );
		printf( "%s-%d:\n", devnm[ dev ], dev );
		printf( "Total seeks = %ld\n\n", total );
	}

	/*
	** clear out pline
	*/
	for ( i = 0; i < LINELEN; i++ )
	{
		if ( i == HSLOP - 1 )
			pline[ i ] = '|';
		else
			pline[ i ] = ' ';
	}
	/*
	** add a null at the end
	*/
	pline[ i ] = '\0';

	/*
	** calculate largest percent
	*/
	if ( total != 0L )
		maxcnt = (long)( ( (float)hist[ 0 ].count / (float)total ) * 100. );
	else
		maxcnt = 0L;

	/*
	** step through the hist array, until a zero count,
	** printing a BLOT for the counts
	*/
	startindex = 0;
	once = 1;
	while ( maxcnt )
	{
		for ( i = startindex; i < NUMCHK; i++ )
		{
			if ( hist[ i ].count == 0L )
				break;

			if ( (long)( ( (float)hist[ i ].count /
				      (float)total ) * 100. ) == maxcnt )
			{
				pline[ HSLOP + ( hist[ i ].startnum / GRAN ) *
								DIVSIZ ] = BLOT;
				startindex = i + 1;
			}
		}

		if ( once || ( ( maxcnt % 10L == 0 ) && maxcnt >= 10L ) )
		{
			sprintf( tmpstr, "%ld%%-", maxcnt );
			strncpy( pline, tmpstr, HSLOP );
			once = 0;
		}
		else
		{
			sprintf( tmpstr, "   |" );
			strncpy( pline, tmpstr, HSLOP );
		}
		printf( "%s\n", pline );
		--maxcnt;
	}

	/*
	** now, print the footer
	*/
	spaces( HSLOP - 1 );
	printf( "+" );
	for ( i = 0; i < LINELEN; i++ )
	{
		if ( i % DIVSIZ == 0 )
			printf( "|" );
		else
			printf( "-" );
	}
	printf( "\n" );
	
	/*
	** print the hundreds line
	*/
	spaces( HSLOP );
	for ( i = HSLOP, j = 0; i < LINELEN; i++, j += GRAN / DIVSIZ )
	{
		if ( i % ( DIVSIZ * GRAN ) == 0 )
		{
			printf( "%d", j / 100 );
		}
		else
			printf( " " );
	}
	printf( "\n" );
	spaces( HSLOP );
	/*
	** print the tens line
	*/
	for ( i = HSLOP, j = 0; i < LINELEN; i++, j += GRAN / DIVSIZ )
	{
		if ( i % ( DIVSIZ * GRAN ) == 0 )
			printf( "%d", ( j % 100 ) / 10 );
		else
			printf( " " );
	}
	printf( "\n" );
	spaces( HSLOP );
	/*
	** print the ones line
	*/
	for ( i = HSLOP, j = 0; i < LINELEN; i++, j += GRAN / DIVSIZ )
	{
		if ( i % ( DIVSIZ * GRAN ) == 0 )
			printf( "%d", j % 10 );
		else
			printf( " " );
	}
	printf( "\n" );

	spaces( LINELEN / 4 );
	if ( type == CYL )
	{
		printf( "cylinder numbers -- divisions every %d cylinders\n",
									GRAN );
	}
	else
	{
		printf( "seek distances -- divisions every %d cylinders\n",
									GRAN );
	}
}

/*
** this routine makes qsort sort in reverse order
*/
compar( x, y )
struct hstruct	*x, *y;
{
	if ( x->count > y->count )
		return( -1 );
	else
		if ( x->count < y->count )
			return( 1 );
	return( 0 );
}

spaces( num )
int	num;
{
	while ( num-- )
		printf( " " );
}
