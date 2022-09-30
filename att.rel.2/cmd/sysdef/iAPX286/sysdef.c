/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/*
** sysdef
**	produce a dfile-like ( config(1m) ) output from a given kernel and
**	master file.
**
**	compile: cc -Ml sysdef.c -lld -o sysdef
*/
#include	<stdio.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/var.h>
#include	<filehdr.h>
#include	<scnhdr.h>
#include	<ldfcn.h>
#include	<fcntl.h>
#include	<nlist.h>
#include	<string.h>

/*
** macro to check if a given char is in a given string
*/
#define	instring( str, c )	( strchr( str, c ) == NULL ? 0 : 1 )

#define	major( x )		( x >> 8 )
#define	minor( x )		( x & 0xFF )

#define	OPEN	'o'		/* open routine exists			*/
#define	CLOSE	'c'		/* close routine exists			*/
#define	READ	'r'		/* read routine exists			*/
#define	WRITE	'w'		/* write routine exists			*/
#define	IOCTL	'i'		/* ioctl routine exists			*/
#define INIT	's'		/* initialization routine exists	*/
#define	FORK	'f'		/* fork routine exists			*/
#define	EXEC	'e'		/* exec routine exists			*/
#define	EXIT	'x'		/* exit routine exists			*/

#define ONCE		'o'		/* only one spec. of device	*/
#define NOCNT		's'		/* suppress device count field	*/
#define REQ		'r'		/* required device		*/
#define	BLOCK		'b'		/* block type device		*/
#define	CHAR		'c'		/* character type device	*/
#define	TTYS		't'		/* device is a tty		*/

#define	MAXLINE		256		/* max size of line in master	*/
#define	MAXNL		200		/* max size of nl array		*/
#define	LDNULL		(LDFILE *)0	/* a null that libld likes	*/
#define	FILENULL	(FILE *)0	/* a null that buf i/o likes	*/
#define	MASTERFILE	"/etc/master"	/* default master file		*/
#define	OSFILE		"/unix"		/* default name for unix	*/

char	*masterfile,			/* file to use for /etc/master	*/
	*osfile;			/* file to use for /unix	*/

LDFILE	*ldptr;				/* file pointer for libld	*/
FILE	*masterfd;			/* file ptr for master file	*/

/*
** namelist struct and a whole bunch of variables that point
** to their locations in nl[]
*/
struct nlist	nl[ MAXNL ], *nlptr, *addentry(), *mesg, *sema, *bdevcnt,
		*bdevsw, *rootdev, *swapdev, *dumpdev, *swplo, *nswap,
		*shmem, *pipedev, *vstruct, *swapmap, *coremap, *callo,
		*handlers, *emul;

char	line[ MAXLINE ];		/* holds line from master	*/

/*
** array of structures to hold useful information
** from the master file
*/
struct masinfo
{
	char	masname[ 10 ];		/* name of device	*/
	int	masvec;			/* vector of device	*/
	char	mashndl[ 10 ];		/* device handlers	*/
	char	mastype[ 10 ];		/* type of device	*/
	char	maspref[ 10 ];		/* prefix of device	*/
	int	masbmaj;		/* major dev number	*/
} masinfo[ 100 ], *masptr;

struct var	v;

long		seekaddr();	/* forward references		*/
char		*bmajsearch();
char		*findroutine();
char		*prefsearch();
struct nlist	*nlsearch();

unsigned long	vtab[ 256 ];	/* place to read in vector table*/

main( argc, argv )
int	argc;
char	**argv;
{
	char	devcnt[ 10 ],	/* constructed name for xxx_cnt	*/
		devintr[ 10 ];	/* constructed name for xxxintr*/
	/*
	** variables that define a line from the master file
	*/
	char	name[ 10 ],	/* name 			*/
		hndl[ 10 ],	/* handlers 			*/
		type[ 10 ],	/* type				*/
		pref[ 10 ],	/* prefix			*/
		decl[ 10 ];	/* structure declaration	*/
	int	vec,		/* vector 			*/
		bmaj,		/* block major			*/
		cmaj,		/* char major			*/
		num;		/* number of instances		*/

	masterfile = MASTERFILE;
	osfile = OSFILE;

	/*
	** figure out what we were called with
	*/
	switch( argc )
	{
		case 3:
			masterfile = argv[ 2 ];
		case 2:
			osfile = argv[ 1 ];
		case 1:
			break;
		default:
			fprintf( stderr, "usage: %s [ opsys [ master ] ]\n",
								argv[ 0 ] );
			exit( 1 );
	}

	/*
	** open operating system file
	*/
	if ( ( ldptr = ldopen( osfile, LDNULL ) ) == LDNULL )
	{
		perror( osfile );
		exit( 1 );
	}

	/*
	** validate magic number
	*/
	if ( TYPE( ldptr ) != I286LMAGIC )
	{
		fprintf( stderr, "0x%x is not a valid iAPX286 magic number\n",
								TYPE( ldptr ) );
		exit( 1 );
	}

	/*
	** open master file
	*/
	if ( ( masterfd = fopen( masterfile, "r" ) ) == FILENULL )
	{
		perror( masterfile );
		exit( 1 );
	}

	/*
	** fill in the namelist structure with all the stuff
	** we know we have to have
	*/
	nlptr = &nl[ 0 ];
	bdevcnt = addentry( "bdevcnt" );
	bdevsw = addentry( "bdevsw" );
	rootdev = addentry( "rootdev" );
	pipedev = addentry( "pipedev" );
	swapdev = addentry( "swapdev" );
	dumpdev = addentry( "dumpdev" );
	swplo = addentry( "swplo" );
	nswap = addentry( "nswap" );
	sema = addentry( "seminfo" );
	shmem = addentry( "shminfo" );
	mesg = addentry( "msginfo" );
	vstruct = addentry( "v" );
	swapmap = addentry( "swapmap" );
	coremap = addentry( "coremap" );
	callo = addentry( "callout" );
	handlers = addentry( "handlers" );
	emul = addentry( "emul" );

	masptr = &masinfo[ 0 ];

	/*
	** fill in the namelist structure with stuff that we find
	** in master file
	*/
	while ( fgets( line, MAXLINE, masterfd ) != (char *)0 )
	{
		/*
		** eat comments
		*/
		if ( line[ 0 ] == '*' )
			continue;

		/*
		** change of section in master -- time to
		** stop this loop
		*/
		if ( line[ 0 ] == '$' )
			break;

		/*
		** break up the master line into variables,
		** and add xxx_cnt, and xxxintr into
		** the namelist array
		*/
		if ( sscanf( line, "%s %d %s %s %s %d %d %d %s", name, &vec,
			hndl, type, pref, &bmaj, &cmaj, &num, decl ) != 8 )
		{
			continue;
		}

		/*
		** add entry into master structure
		*/
		masterentry( name, vec, type, hndl, pref, bmaj );

		/*
		** construct names to search for.
		**
		** if suppress count is not in the type field,
		** look for xxx_cnt, otherwise, look for 
		** another address that we can look for
		*/
		if ( instring( type, NOCNT ) )
		{
			addentry( findroutine( hndl, pref ) );
		}
		else
		{
			strcpy( devcnt, pref );
			strcat( devcnt, "_cnt" );
			addentry( devcnt );
		}
		strcpy( devintr, pref );
		strcat( devintr, "intr" );
		addentry( devintr );
	}

	/*
	** make sure that the last entry in the namelist structure
	** is null
	*/
	addentry( "" );

	/*
	** finally, fill in the namelist structure
	*/
	if ( nlist( osfile, nl ) == -1 )
	{
		perror( "nlist" );
		exit( 1 );
	}

#ifdef DEBUG
	/*
	** print out the nl array
	*/
	{
		struct nlist *ptr;
		printf( "name\t\tvalue\t\t\ttype\n" );
		for ( ptr = &nl[ 0 ]; ptr->n_name[ 0 ] != 0; ptr++ )
		{
			printf( "%s%s", ptr->n_name,
				strlen( ptr->n_name ) >= 8 ? "\t" : "\t\t" );

			printf( "0x%*.*lx\t\t", sizeof( ptr->n_value ) * 2,
				sizeof( ptr->n_value ) * 2, ptr->n_value );

			printf( "0x%*.*x\n", sizeof( ptr->n_type ) * 2,
				sizeof( ptr->n_type ) * 2, ptr->n_type );
		}
	}
#endif

	vectable();		/* read in vector table	*/
	i286devs();
	pseudo_devs();
	sysdevs();
	tunables();
}

/*
** addentry
**	add an entry to the namelist structure array
*/
struct nlist *
addentry( nam )
char	*nam;
{
	extern char	*malloc();

	if ( nlptr >= &nl[ MAXNL ] )
	{
		fprintf( stderr, "internal name list overflow\n" );
		exit( 1 );
	}

	if ( ( nlptr->n_name = malloc( strlen( nam ) + 1 ) ) == NULL )
	{
		fprintf( stderr, "cannot allocate memory for name list\n" );
		exit(1);
	}

	strcpy( nlptr->n_name, nam );
	nlptr->n_type = 0;
	nlptr->n_value = 0;
	return( nlptr++ );	/* increment pointer */
}

/*
** i286devs
**	print out the information about real devices
*/
i286devs()
{
	struct masinfo	*ptr;
	char		ctmp[ 20 ];
	char		intrname[ 20 ];
	int		nocnt;
	struct nlist	*entry;
	struct nlist	*intentry;
	int		cnt;

	printf( "*\n* MULTIBUS\n* " );
	printf( "name\tvector\tcount\n*\n" );

	/*
	** traverse through the masinfo array, and find an address
	** that should be is the kernel if this device is configured
	** in.
	*/
	for ( ptr = &masinfo[ 0 ]; ptr != masptr; ptr++ )
	{
		/*
		** ignore pseudo-devs for now
		*/
		if ( ptr->masvec == 0 )
			continue;

		/*
		** don't print console
		*/
		if ( strcmp( ptr->masname, "con" ) == 0 )
			continue;

		if ( instring( ptr->mastype, NOCNT ) )
		{
			strcpy( ctmp, findroutine( ptr->mashndl, ptr->maspref ) ); 
			++nocnt;
		}
		else
		{
			strcpy( ctmp, ptr->maspref );
			strcat( ctmp, "_cnt" );
			nocnt = 0;
		}

		/*
		** try to find the item in the namelist
		*/
		if ( entry = nlsearch( ctmp ) )
		{
			printf( "%s\t", prefsearch( ptr->maspref ) );
			strcpy( intrname, ptr->maspref );
			strcat( intrname, "intr" );
			if ( ( intentry = nlsearch( intrname ) ) == (struct nlist *)0 )
			{
				printf( "OOPS\t" );
			}
			else
			{
				printf( "%d\t", getvect( intentry->n_value ) );
			}

			if ( nocnt == 0 )
			{
				efseek( ldptr, seekaddr( entry ), 0 );
				efread( &cnt, sizeof( cnt ), 1, ldptr );
				printf( "%d\n", cnt );
			}
			else
			{
				printf( "1\n" );
			}
		}
	}
}

/*
** pseudo_devs
**	print out the information about pseudo devices
*/
pseudo_devs()
{
	struct masinfo	*ptr;
	char		ctmp[ 20 ];
	int		nocnt;
	struct nlist	*entry;
	int		cnt;

	printf( "*\n* Pseudo Devices\n* " );
	printf( "name\t\tcount\n*\n" );

	/*
	** traverse through the masinfo array, and find an address
	** that should be is the kernel if this device is configured
	** in.
	*/
	for ( ptr = &masinfo[ 0 ]; ptr != masptr; ptr++ )
	{
		/*
		** ignore everything but pseudo-devs
		*/
		if ( ptr->masvec )
			continue;
		/*
		** don't print shared mem, msg, or semaphores
		*/
		if ( strcmp( ptr->masname, "shmem" ) == 0 ||
			strcmp( ptr->masname, "sema" ) == 0 ||
			strcmp( ptr->masname, "mesg" ) == 0 ||
			strcmp( ptr->masname, "memory" ) == 0 ||
			strcmp( ptr->masname, "tty" ) == 0 ||
			strcmp( ptr->masname, "errlog" ) == 0 )
		{
			continue;
		}

		/*
		** special case 287 emulator
		*/
		if ( strcmp( ptr->masname, "emul" ) == 0 )
		{
			if ( emul->n_value )
				printf( "emul\t0\t1\n" );
			continue;
		}

		if ( instring( ptr->mastype, NOCNT ) )
		{
			strcpy( ctmp, findroutine( ptr->mashndl, ptr->maspref ) ); 
			++nocnt;
		}
		else
		{
			strcpy( ctmp, ptr->maspref );
			strcat( ctmp, "_cnt" );
			nocnt = 0;
		}

		/*
		** try to find the item in the namelist
		*/
		if ( entry = nlsearch( ctmp ) )
		{
			printf( "%s\t0\t", prefsearch( ptr->maspref ) );

			if ( nocnt == 0 )
			{
				efseek( ldptr, seekaddr( entry ), 0 );
				efread( &cnt, sizeof( cnt ), 1, ldptr );
				printf( "%d\n", cnt );
			}
			else
			{
				printf( "1\n" );
			}
		}
	}
}

/*
** sysdevs
**	printf out the information about system devices
*/
sysdevs()
{
	unsigned int	i;
	unsigned long	l;

	printf( "*\n* System Devices\n* " );
	printf( "name\tdevice\tminor\tswplo\tnswap\n*\n" );

	efseek( ldptr, seekaddr( rootdev ), 0 );
	efread( &i, sizeof( i ), 1, ldptr );
	printf( "root\t%s\t%d\n", bmajsearch( major( i ) ), minor( i ) );

	efseek( ldptr, seekaddr( pipedev ), 0 );
	efread( &i, sizeof( i ), 1, ldptr );
	printf( "pipe\t%s\t%d\n", bmajsearch( major( i ) ), minor( i ) );

	efseek( ldptr, seekaddr( swapdev ), 0 );
	efread( &i, sizeof( i ), 1, ldptr );
	printf( "swap\t%s\t%d\t", bmajsearch( major( i ) ), minor( i ) );
	efseek( ldptr, seekaddr( swplo ), 0 );
	efread( &l, sizeof( l ), 1, ldptr );
	printf( "%ld\t", l );
	efseek( ldptr, seekaddr( nswap ), 0 );
	efread( &i, sizeof( i ), 1, ldptr );
	printf( "%d\n", i );

	efseek( ldptr, seekaddr( dumpdev ), 0 );
	efread( &i, sizeof( i ), 1, ldptr );
	printf( "dump\t%s\t%d\n", bmajsearch( major( i ) ), minor( i ) );
}

/*
** tunables
**	print out info about the tunable parameters
*/
tunables()
{
	printf( "*\n* Tunable Parameters\n* " );
	printf( "name\t\tvalue\n*\n" );

	efseek( ldptr, seekaddr( vstruct ), 0 );
	efread( &v, sizeof( v ), 1, ldptr );

	printf( "buffers\t%11d\n", v.v_buf );
	printf( "calls\t%11d\n", v.v_call );
	printf( "inodes\t%11d\n", v.v_inode );
	printf( "files\t%11d\n", v.v_file );
	printf( "mounts\t%11d\n", v.v_mount );
	printf( "procs\t%11d\n", v.v_proc );
	printf( "texts\t%11d\n", v.v_text );
	printf( "clists\t%11d\n", v.v_clist );
	printf( "sabufs\t%11d\n", v.v_sabuf );
	printf( "maxproc\t%11d\n", v.v_maxup );
	printf( "coremap\t%11d\n", v.v_cmap );
	printf( "swapmap\t%11d\n", v.v_smap );
	printf( "hashbuf\t%11d\n", v.v_hbuf );
	printf( "physbuf\t%11d\n", v.v_pbuf );
	printf( "mesg\t%11d\n", mesg->n_value ? 1 : 0 );
	printf( "sema\t%11d\n", sema->n_value ? 1 : 0 );
	printf( "shmem\t%11d\n", shmem->n_value ? 1 : 0 );
}

/*
** seekaddr
**	given a pointer to a namelist structure, return
**	the location of the item in the osfile. This
**	pointer is suitable to use for lseeks
*/
long
seekaddr( nmptr )
struct nlist *nmptr;
{
	SCNHDR	scnhdr;		/* place to read in section hdr	*/

	if ( ldshread( ldptr, nmptr->n_scnum, &scnhdr ) == FAILURE )
	{
		perror( "ldshread" );
		exit( 1 );
	}

	return( (long)( scnhdr.s_scnptr + (int)nmptr->n_value ) );
}

/*
** masterentry
**	add some useful information to the master structure
*/
masterentry( nam, vector, typ, hdl, pre, maj )
char	*nam,
	*typ,
	*pre,
	*hdl;
int	vector;
int	maj;
{
	strcpy( masptr->masname, nam );
	strcpy( masptr->mastype, typ );
	strcpy( masptr->maspref, pre );
	strcpy( masptr->mashndl, hdl );
	masptr->masbmaj = maj;
	masptr->masvec = vector;

	++masptr;
}

/*
** bmajsearch
**	given a block device major number, returns a char ptr
**	to the name found, or the char string "unknown"
*/
char	*
bmajsearch( maj )
int	maj;
{
	struct masinfo	*ptr;

	for ( ptr = &masinfo[ 0 ]; ptr < masptr; ptr++ )
	{
		if ( instring( ptr->mastype, BLOCK ) )
		{
			if ( ptr->masbmaj == maj )
			{
				return( ptr->masname );
			}
		}
	}
	return( "unknown" );
}

/*
** vectable
**	read in the interrupt vector table
*/
vectable()
{
	efseek( ldptr, seekaddr( handlers ), 0 );
	efread( vtab, sizeof( vtab ), 1, ldptr );
}

/*
** getvect
**	returns the interrupt vector for a given
** 	interrupt routine address
** NOTE:
**	this routine will return 0 if it cannot
** 	find the vector. This assumes that
**	nobody put a i/o handler interrupt at vector
**	0 ( which is intel reserved for divide by zero,
**	anyway )
*/
getvect( addr )
unsigned long	addr;
{
	int	i;

	for ( i = 0; i < 256; i++ )
	{
		if ( vtab[ i ] == addr )
		{
			return( i );
		}
	}
	return( 0 );
}

/*
** findroutine
**	looks at hdl and constructs a string of
**	the form:
**		pre{open,close,read,write,ioctl}
**	based on the first routine it finds that
**	hdl contains.
*/
char *
findroutine( hdl, pre )
char	*hdl,
	*pre;
{
	static char	tmp[ 20 ];

	strcpy( tmp, pre );

	/*
	** Sorry for the way this routine is formatted,
	** but I think its easier to read this way.
	** All I really want is ONE of the statement to happen.
	*/
	if ( instring( hdl, OPEN ) ) strcat( tmp, "open" );
	else
	if ( instring( hdl, CLOSE ) ) strcat( tmp, "close" );
	else
	if ( instring( hdl, READ ) ) strcat( tmp, "read" );
	else
	if ( instring( hdl, WRITE ) ) strcat( tmp, "write" );
	else
	if ( instring( hdl, IOCTL ) ) strcat( tmp, "ioctl" );
	else
	if ( instring( hdl, INIT ) ) strcat( tmp, "init" );
	else
	if ( instring( hdl, FORK ) ) strcat( tmp, "fork" );
	else
	if ( instring( hdl, EXEC ) ) strcat( tmp, "exec" );
	else
	if ( instring( hdl, EXIT ) ) strcat( tmp, "exit" );

	return( tmp );
}

/*
** prefsearch
**	given a prefix, return a ptr to the
**	real name of the device, or "unknown"
*/
char *
prefsearch( p )
char	*p;
{
	struct masinfo	*ptr;

	for ( ptr = &masinfo[ 0 ]; ptr < masptr; ptr++ )
	{
		if ( strcmp( ptr->maspref, p ) == 0 )
			return( ptr->masname );
	}
	return( "unknown" );
}

/*
** nlsearch
**	given a name, returns a pointer to the namelist structure
**	that the name has for it, or 0L if name is not found
*/
struct nlist *
nlsearch( name )
char	*name;
{
	static struct nlist	*ptr;

	for ( ptr = &nl[ 0 ]; ptr->n_name != NULL; ptr++ )
	{
		if ( strcmp( ptr->n_name, name ) == 0 )
			if ( ptr->n_value != 0L )
				return( ptr );
	}
	return( (struct nlist *)0 );
}

/*
** efseek
**	FSEEK with error checking
*/
efseek( stream, offset, whence )
LDFILE	*stream;
long	offset;
int	whence;
{
	if ( FSEEK( stream, offset, whence ) != OKFSEEK )
	{
		perror( "fseek" );
		exit( 1 );
	}
}

/*
** efread
**	FREAD with error checking
*/
efread( ptr, size, nitems, stream )
char	*ptr;
int	size,
	nitems;
LDFILE	*stream;
{
	if ( FREAD( ptr, size, nitems, stream ) == FAILURE )
	{
		perror( "fread" );
		exit( 1 );
	}
}
