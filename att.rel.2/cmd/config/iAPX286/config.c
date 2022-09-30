/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*
** config for iAPX 286
*/
#include "stdio.h"

char	*strchr();	/* forward reference	*/

/*
** macro to check if a given char is in a given string
*/
#define	instring( str, c )	( strchr( str, c ) == NULL ? 0 : 1 )

#define	TSIZE	64		/* max configuration table */
#define	DSIZE	64		/* max device table */
#define	ASIZE	20		/* max alias table */
#define MSIZE	100		/* max address map table */
#define	PSIZE	128		/* max keyword table */
#define	BSIZE	16		/* max block device table */
#define	CSIZE	64		/* max character device table */

/*
** chars that go into type[]
*/
#define ONCE	'o'		/* allow only one spec. of device	*/
#define NOCNT	's'		/* suppress device count field		*/
#define REQ	'r'		/* required device			*/
#define	BLOCK	'b'		/* block type device			*/
#define	CHAR	'c'		/* character type device		*/
#define	TTYS	't'		/* device is a tty			*/

/*
** chars that go into mask[]
*/
#define	OPEN	'o'		/* open routine exists			*/
#define	CLOSE	'c'		/* close routine exists			*/
#define	READ	'r'		/* read routine exists			*/
#define	WRITE	'w'		/* write routine exists			*/
#define	IOCTL	'i'		/* ioctl routine exists			*/
#define INIT	's'		/* initialization routine exists	*/
#define	FORK	'f'		/* fork routine exists			*/
#define	EXEC	'e'		/* exec routine exists			*/
#define	EXIT	'x'		/* exit routine exists			*/

struct	t	{
	char	*devname;	/* pointer to device name */
	short	vector;		/* interrupt vector location */
	char	mask1[ 10 ];	/* letters indicating existing handlers	*/
	char	*type1;		/* letters indicating device type	*/
	char	*handler;	/* pointer to interrupt handler */
	short	blk;		/* major device number if block type device */
	short	chr;		/* major device number if char. type device */
	short	mlt;		/* number of devices on controller */
	short	count;		/* sequence number for this dev	*/
} table	[TSIZE];

struct	t2
{
	char	dev[ 9 ];	/* device name				*/
	short	vect;		/* interrupt vector			*/
	char	mask[ 10 ];	/* letters indicating existing handlers	*/
	char	type[ 7 ];	/* letters indicating device type	*/
	char	hndlr[ 5 ];	/* handler name				*/
	short	block;		/* major dev number if block device	*/
	short	charr;		/* major dev number if character device	*/
	short	mult;		/* max number of devices per controller	*/
	short	dcount;		/* number of controllers present	*/
	char	entries[ 3 ][ 9 ];	/* conf. structure definitions	*/
	short	acount;		/* number of devices			*/
} devinfo[ DSIZE ];

struct	t3	{
	char	new[9];		/* alias of device */
	char	old[9];		/* reference name of device */
} alias[ASIZE];

struct	t4	{
	char	indef[21];		/* input parameter keyword */
	char	oudef[21];		/* output definition symbol */
	char	value[21];		/* actual parameter value */
	char	defval[21];		/* default parameter value */
} parms[PSIZE];

struct	t2	*bdevices[BSIZE];	/* pointers to block devices */
struct	t2	*cdevices[CSIZE];	/* pointers to char. devices */
struct	t	*p;			/* configuration table pointer */
struct	t2	*q;			/* master device table pointer */
struct	t3	*r;			/* alias table pointer */
struct	t4	*kwdptr;		/* keyword table pointer */
struct	t	*locate();
struct	t2	*find();
struct	t4	*lookup();
char	*uppermap();

short	eflag	= 0;	/* error in configuration */
short	tflag	= 0;	/* table flag */
short	bmax	= -1;	/* dynamic max. major device number for block device */
short	cmax	= -1;	/* dynamic max. major device number for char. device */
short	blockex	= 0;	/* dynamic end of block device table */
short	charex	= 0;	/* dynamic end of character device table */
short	subtmp	= 0;	/* temporary; for subscripting */
short	abound	= -1;	/* current alias table size */
short	dbound	= -1;	/* current device table size */
short	tbound	= -1;	/* current configuration table size */
short	pbound	= -1;	/* current keyword table size */
short	rtmaj	= -1;	/* major device number for root device */
short	swpmaj	= -1;	/* major device number for swap device */
short	pipmaj	= -1;	/* major device number for pipe device */
short	dmpmaj	= -1;	/* major device number for dump device */
short	rtmin	= -1;	/* minor device number for root device */
short	swpmin	= -1;	/* minor device number for swap device */
short	pipmin	= -1;	/* minor device number for pipe device */
short	dmpmin	= -1;	/* minor device number for dump device */
long	swplo	= -1;	/* low disc address for swap area */
int	nswap	= -1;	/* number of disc blocks in swap area */
FILE	*confhp;	/* configuration include file pointer */
FILE	*fdconf;	/* configuration file pointer */
FILE	*fdhnd;		/* low core file pointer */
struct t *dmppt;	/* pointer to dump entry */
char	*mfile;		/* master device file */
char	*cfile;		/* output configuration table file */
char	*hfile;		/* output configuration define file */
char	*lfile;		/* output low core file */
char	*infile;	/* input configuration file */
char	line[101];	/* input line buffer area */
char	initbf[513];	/* initialization routine buffer */
char	forkbf[513];	/* fork routine buffer */
char	execbf[513];	/* exec routine buffer */
char	exitbf[513];	/* exit routine buffer */
char	xbf[111];	/* buffer for external symbol definitions */
char	d[9];		/* buffer area */
char	capitals[9];	/* buffer area */
short	vec;		/* specified interrupt vector */
short	ml;		/* specified device multiplicity */

char	*opterror =		/* option error message */
{
	"Option re-specification\n"
};

char	dummy[ 9 ];

/*
** array of device interrupt routine names to write to
** handlers.c
*/
char	vct[ 256 ][ 9 ];


main(argc,argv)
int argc;
char *argv[];
{
	register i, l;
	register FILE *fd;
	int number;
	char input[21], bf[9], c, symbol[21];
	char *argv2;
	struct t *pt;
	argc--;
	argv++;
	argv2 = argv[0];
	argv++;
/*
 * Scan off the 'c', 'h', 'l', 'm', and 't' options.
 */
	while ((argc > 1) && (argv2[0] == '-')) {
		for (i=1; (c=argv2[i]) != NULL; i++) switch (c) {
		case 'c':
			if (cfile) {
				printf(opterror);
				exit(1);
			}
			cfile = argv[0];
			argv++;
			argc--;
			break;
		case 'h':
			if (hfile) {
				printf(opterror);
				exit(1);
			}
			hfile = argv[0];
			argv++;
			argc--;
			break;
		case 'l':
			if (lfile) {
				printf(opterror);
				exit(1);
			}
			lfile = argv[0];
			argv++;
			argc--;
			break;
		case 'm':
			if (mfile) {
				printf(opterror);
				exit(1);
			}
			mfile = argv[0];
			argv++;
			argc--;
			break;
		case 't':
			tflag++;
			break;

		default:
			printf("Unknown option\n");
			exit(1);
		}
		argc--;
		argv2 = argv[0];
		argv++;
	}
	if (argc != 1) {
		printf("Usage: config [-t][-l file][-c file][-m file] file\n");
		exit(1);
	}
/*
 * Set up defaults for unspecified files.
 */
	if (cfile == 0)
		cfile = "conf.c";
	if (hfile == 0)
		hfile = "config.h";
	if (lfile == 0)
		lfile = "handlers.c";
	if (mfile == 0)
		mfile = "/etc/master";
	infile = argv2;
	fd = fopen (infile,"r");
	if (fd == NULL) {
		printf("Open error for file -- %s\n",infile);
		exit(1);
	}
/*
 * Open configuration file and set modes.
 */
	fdconf = fopen (cfile,"w");
	if (fdconf == NULL) {
		printf("Open error for file -- %s\n",cfile);
		exit(1);
	}
	chmod (cfile,0644);
/*
 * Write configuration define file.
 */
	confhp = fopen (hfile,"w");
	if (confhp == NULL) {
		printf("Open error for file -- %s\n",hfile);
		exit(1);
	}
	chmod (hfile,0644);
	fprintf(confhp,"/* Configuration Definition */\n");
/*
 * Print configuration file heading.
 */
	fprintf(fdconf,"/*\n *  Configuration information\n */\n\n");
	p = table;
/*
 * Read in the master device table and the alias table.
 */
	file();
/*
 * Start scanning the input.
 */
	while (fgets(line,100,fd) != NULL) {
		if (line[0] == '*')
			continue;
		l = sscanf(line,"%20s",input);
		if (l == 0) {
			error("Incorrect line format #1");
			continue;
		}
		if (equal(input,"root")) {
/*
 * Root device specification
 */
			l = sscanf(line,"%8s%8s%d",dummy,bf,&number);
			if (l != 3) {
				error("Incorrect line format #2");
				continue;
			}
			if ((pt=locate(bf)) == 0) {
				error("No such device");
				continue;
			}
			if ( ! instring(pt->type1, BLOCK)) {
				error("Not a block device #1");
				continue;
			}
			if (number > 255) {
				error("Invalid minor device number");
				continue;
			}
			if (rtmin >= 0) {
				error("Root re-specification");
				continue;
			}
			rtmin = number;
			rtmaj = pt->blk;
		} else
		    if (equal(input,"swap")) {
/*
 * Swap device specification
 */
			l = sscanf(line,"%8s%8s%d%ld%d", dummy,
					bf,&number,
					&swplo,&nswap);
			if (l != 5) {
				error("Incorrect line format #3");
				continue;
			}
			if ((pt=locate(bf)) == 0) {
				error("No such device");
				continue;
			}
			if ( ! instring(pt->type1, BLOCK)) {
				error("Not a block device #2");
				continue;
			}
			if (number > 255) {
				error("Invalid minor device number");
				continue;
			}
			if (swpmin >= 0) {
				error("Swap re-specification");
				continue;
			}
			if (nswap < 1) {
				error("Invalid nswap");
				continue;
			}
			if (swplo < 0) {
				error("Invalid swplo");
				continue;
			}
			swpmin = number;
			swpmaj = pt->blk;
		} else
		    if (equal(input,"pipe")) {
/*
 * Pipe device specification
 */
			l = sscanf(line,"%8s%8s%d",dummy,bf,&number);
			if (l != 3) {
				error("Incorrect line format #4");
				continue;
			}
			if ((pt=locate(bf)) == 0) {
				error("No such device");
				continue;
			}
			if ( ! instring(pt->type1, BLOCK)) {
				error("Not a block device #3");
				continue;
			}
			if (number > 255) {
				error("Invalid minor device number");
				continue;
			}
			if (pipmin >= 0) {
				error("Pipe re-specification");
				continue;
			}
			pipmin = number;
			pipmaj = pt->blk;
		} else
		    if (equal(input,"dump")) {
/*
 * Dump device specification
 */
			l = sscanf(line,"%8s%8s%d",dummy,bf,&number);
			if (l != 3) {
				error("Incorrect line format #5");
				continue;
			}
			if ((pt=locate(bf)) == 0) {
				error("No such device");
				continue;
			}
			if ( ! instring(pt->type1, BLOCK)) {
				error("Not a block device #4");
				continue;
			}
			if (number > 255) {
				error("Invalid minor device number");
				continue;
			}
			if (dmpmin >= 0) {
				error("Dump re-specification");
				continue;
			}
			dmpmin = number;
			dmpmaj = pt->blk;
			dmppt = pt;
		} else {
/*
 * Device or parameter specification other than root, swap, pipe, or dump.
 */
			kwdptr = lookup(input);
			if (kwdptr) {
				l = sscanf(line,"%20s%20s",input,symbol);
				if (l != 2) {
					error("Incorrect line format #6");
					continue;
				}
				if (strlen(kwdptr->value)) {
					error("Parameter re-specification");
					continue;
				}
				strcpy(kwdptr->value,symbol);
				continue;
			}
			l = sscanf(line,"%8s%hd%hd",d,&vec,&ml);
			if (l < 2) {
				error("Incorrect line format #7");
				continue;
			}
/*
 * Does such a device exist, and if so, do we allow its specification?
 */
			if ((q=find(d)) == 0) {
				error("No such device");
				continue;
			}
			if (instring(q->type, ONCE) && (locate(d))) {
				error("Only one specification allowed");
				continue;
			}
/*
 * check validity of vector
 */
			if (vec != 0) {
				if (vec < 32 || vec > 255) {
					error("Vector not in 32-255 range");
					continue;
				}
			}
/*
 * See if the optional device multiplier was specified, and if so, see
 * if it is a valid one.
 */
			if (l == 3) {
				if ((ml > q->mult) || (ml < 1)) {
					error("Invalid device multiplier");
					continue;
				}
			}
/*
 * Multiplier not specified, so take a default value from master device table.
 */
			else
				ml = q->mult;
/*
 * Fill in the contents of the configuration table node for this device.
 */
			enterdev(vec,ml,d);
		}
	}
/*
 * Make sure that the root, swap, pipe and dump devices were specified.
 * Set up default values for tunable things where applicable.
 */
	if (rtmaj < 0) {
		printf("root device not specified\n");
		eflag++;
	}
	if (swpmaj < 0) {
		printf("swap device not specified\n");
		eflag++;
	}
	if (pipmaj < 0) {
		printf("pipe device not specified\n");
		eflag++;
	}
	if (dmpmaj < 0) {
		printf("dump device not specified\n");
		eflag++;
	}
	for (kwdptr=parms; kwdptr<= &parms[pbound]; kwdptr++) {
		if (strlen(kwdptr->value) == 0) {
			if (strlen(kwdptr->defval) == 0) {
				printf("%s not specified\n",kwdptr->indef);
				eflag++;
			}
			strcpy(kwdptr->value,kwdptr->defval);
		}
	}
/*
 * See if configuration is to be terminated.
 */
	if (eflag) {
		printf("\nConfiguration aborted.\n");
		exit (1);
	}
/*
 * Configuration is okay, so write the two files and quit.
 */
	for (q=devinfo; q<= &devinfo[dbound]; q++) {
		if (instring(q->type, REQ)) {
			if (!(locate(q->dev))) {
				enterdev(0,q->mult,q->dev);
			}
		}
	}

	prtcdef();
	prtconf();
	prthnd();
	exit(0);
}

/*
 * This routine writes out the handler array file.
 */
prthnd()
{
	register int i;
	register struct t *ptr;

/*
 * Open handler array file and set modes.
 */
	fdhnd = fopen (lfile,"w");
	if (fdhnd == NULL) {
		printf("Open error for file -- %s\n",lfile);
		exit(1);
	}
	chmod (lfile,0644);
/*
 * Print some headings.
 */
	fprintf(fdhnd, "/* Table of Interrupt Vectors */\n\n");
	fprintf(fdhnd, "int	strayint();	/* forward reference */\n\n");
	for (ptr = table; ptr->devname != 0; ptr++) {
		if (ptr->vector)
			fprintf(fdhnd,"extern %sintr();\n", ptr->handler);
	}

	fprintf(fdhnd, "int	(*handlers[])() =\n{\n");

	for (ptr = table; ptr->devname != 0; ptr++) {
		if (ptr->vector)
			strcpy(vct[ ptr->vector ], ptr->handler);
	}
	
	for (i = 0; i < 256; i++) {
		if (vct[ i ][ 0 ] != (char)0) {
			fprintf(fdhnd, "	%sintr,		/* %d	*/\n",
					vct[ i ], i);
		} else
			fprintf(fdhnd,"	strayint,	/* %d	*/\n", i);
	}
	fprintf(fdhnd,"};\n");
}

/*
 * This routine is used to read in the master device table and the
 * alias table.  In the master device file, these two tables are separated
 * by a line that contains an asterisk in column 1.
 */
file()
{
	register l;
	register FILE *fd;
	fd = fopen(mfile,"r");
	if (fd == NULL) {
		printf("Open error for file -- %s\n",mfile);
		exit(1);
	}
	q = devinfo;
	
	while (fgets(line,100,fd) != NULL) {
/*
 * Check for the delimiter that indicates the beginning of the
 * alias table entries.
 */
		if (line[0] == '$')
			break;
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		dbound++;
		if (dbound == DSIZE) {
			printf("Device table overflow\n");
			exit(1);
		}
		l = sscanf(line,"%8s%hd%10s%6s%4s%hd%hd%hd%8s%8s%8s",
			q->dev, &(q->vect), q->mask, q->type,
			q->hndlr, &(q->block), &(q->charr), &(q->mult),
			q->entries[0],q->entries[1],q->entries[2]);

		if (l < 8) {
			printf("%s\nDevice parameter count = %d\n",line,l);
			exit(1);
		}
/*
 * Update the ends of the block and character device tables.
 */
		if (instring(q->type, BLOCK))
			if ((blockex = max(blockex,q->block)) >= BSIZE) {
				printf("%s\nBad major device number\n",line);
				exit(1);
			}
		if (instring(q->type, CHAR))
			if ((charex = max(charex,q->charr)) >= CSIZE) {
				printf("%s\nBad major device number\n",line);
				exit(1);
			}
		q++;
	}
	r = alias;
	while (fgets(line,100,fd) != NULL) {
/*
 * Check for the delimiter that indicates the beginning of the
 * keyword table entries.
 */
		if (line[0] == '$')
			break;
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		abound++;
		if (abound == ASIZE) {
			printf("Alias table overflow\n");
			exit(1);
		}
		l = sscanf(line,"%8s%8s",r->new,r->old);
		if (l < 2) {
			printf("%s\nAlias parameter count\n",line);
			exit(1);
		}
		r++;
	}
	kwdptr = parms;
	while (fgets(line,100,fd) != NULL) {
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		pbound++;
		if (pbound == PSIZE) {
			printf("Keyword table overflow\n");
			exit(1);
		}
		l = sscanf(line,"%20s%20s%20s",kwdptr->indef,
			kwdptr->oudef,kwdptr->defval);
		if (l < 2) {
			printf("%s\nTunable parameter count\n",line);
			exit(1);
		}
		if (l == 2)
			*(kwdptr->defval) = NULL;
		kwdptr++;
	}
}

equal(s1,s2)
register char *s1, *s2;
{
	while (*s1++ == *s2) {
		if (*s2++ == 0)
			return(1);
	}
	return(0);
}

/*
 * This routine is used to print a configuration time error message
 * and then set a flag indicating a contaminated configuration.
 */
error(message)
char *message;
{
	printf("%s%s\n\n",line,message);
	eflag++;
}

/*
 * This routine is used to search through the master device table for
 * some specified device.  If the device is found, we return a pointer to
 * the device.  If the device is not found, we search the alias table for
 * this device.  If the device is not found in the alias table, we return a
 * zero.  If the device is found, we change its name to the reference name of
 * the device and re-initiate the search for this new name in the master
 * device table.
 */
struct t2 *
find(device)
char *device;
{
	register struct t2 *q;
	register struct t3 *r;
	for (;;) {
		for (q=devinfo; q<= &devinfo[dbound]; q++) {
			if (equal(device,q->dev))
				return(q);
		}
		for (r=alias; r<= &alias[abound]; r++) {
			if (equal(device,r->new)) {
				device = r->old;
				break;
			}
		}
		if (r > &alias[abound])
			return(0);
	}
}

/*
 * This routine is used to set the character and/or block table pointers
 * to point to an entry of the master device table.
 */
setq()
{
	register struct t2 *ptr;
	int	ischar,
		isblock,
		isboth;

	isblock = instring(q->type, BLOCK);
	ischar = instring(q->type, CHAR);
	isboth = isblock & ischar;

	if ( ! (isblock | ischar))
		return;

	if (isboth) {
		subtmp = q->charr;
		ptr = cdevices[subtmp];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				charex++;
				if (charex == CSIZE) {
					printf("Character table overflow\n");
					exit(1);
				}
				q->charr = subtmp = charex;
			}
		}
		cdevices[subtmp] = q;
		cmax = max (cmax,subtmp);
		subtmp = q->block;
		ptr = bdevices[subtmp];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				blockex++;
				if (blockex == BSIZE) {
					printf("Block table overflow\n");
					exit(1);
				}
				q->block = subtmp = blockex;
			}
		}
		bdevices[subtmp] = q;
		bmax = max (bmax,subtmp);
	} else {
		if (ischar) {
			subtmp = q->charr;
			ptr = cdevices[subtmp];
			if (ptr) {
				if (!equal(ptr->dev,q->dev)) {
					charex++;
					if (charex == CSIZE) {
						printf("Character ");
						printf("table overflow\n");
						exit(1);
					}
					q->charr = subtmp = charex;
				}
			}
			cdevices[subtmp] = q;
			cmax = max (cmax,subtmp);
		} else {
			/*
			** better damn well be a block device
			*/
			subtmp = q->block;
			ptr = bdevices[subtmp];
			if (ptr) {
				if (!equal(ptr->dev,q->dev)) {
					blockex++;
					if (blockex == BSIZE) {
						printf("Block table ");
						printf("overflow\n");
						exit(1);
					}
					q->block = subtmp = blockex;
				}
			}
			bdevices[subtmp] = q;
			bmax = max (bmax,subtmp);
		}
	}
}

/*
 * This routine writes out the configuration define file (include file.)
 */
prtcdef()
{
	for (kwdptr=parms; kwdptr<= &parms[pbound]; kwdptr++) {
		fprintf(confhp,"#define\t%s\t%s\n",
			kwdptr->oudef, kwdptr->value);
	}
	fclose(confhp);
	return;
}

/*
 * This routine writes out the configuration file (C program.)
 */
prtconf()
{
	register i, j, counter;
	struct t2 *ptr;
/*
 * Print some headings.
 */
	fprintf(fdconf,"\n");
	fprintf(fdconf,"#include\t\"%s\"\n",hfile);
	fprintf(fdconf,"#include\t\"sys/param.h\"\n");
	fprintf(fdconf,"#include\t\"sys/types.h\"\n");
	fprintf(fdconf,"#include\t\"sys/sysmacros.h\"\n");
	fprintf(fdconf,"#include\t\"sys/space.h\"\n");
	fprintf(fdconf,"#include\t\"sys/io.h\"\n");
	fprintf(fdconf,"#include\t\"sys/conf.h\"\n");

	fprintf(fdconf,"extern nodev(), nulldev();\n");

/*
 * Search the configuration table and generate an extern statement for
 * any routines that are needed.
 */
	for (q = devinfo; q <= &devinfo[dbound]; q++) {
		if (! instring(q->type, REQ)) {
			for (p = table; p <= &table[tbound]; p++) {
				if (equal(q->dev,p->devname))
					goto doit;
			}
			{
			struct t4 *tmp;
			if ((tmp = lookup(q->dev)) != 0) {
				if (atoi(tmp->value) != 0 )
					goto doit;
			}
			}
			continue;
		}
doit:

		sprintf(xbf,"extern ");
		if (instring(q->mask, OPEN)) {
			strcat (xbf,q->hndlr);
			strcat (xbf,"open(), ");
		}
		if (instring(q->mask, CLOSE)) {
			strcat (xbf,q->hndlr);
			strcat (xbf,"close(), ");
		}
		if (instring(q->mask, READ)) {
			strcat (xbf,q->hndlr);
			strcat (xbf,"read(), ");
		}
		if (instring(q->mask, WRITE)) {
			strcat (xbf,q->hndlr);
			strcat (xbf,"write(), ");
		}
		if (instring(q->mask, IOCTL)) {
			strcat (xbf,q->hndlr);
			strcat (xbf,"ioctl(), ");
		}
		if (instring(q->type, BLOCK)) {
			strcat (xbf,q->hndlr);
			strcat (xbf,"strategy(), ");
			strcat (xbf,q->hndlr);
			strcat (xbf,"print(), ");
		}
		if (instring(q->mask, INIT)) {
			strcat (xbf,q->hndlr);
			strcat (xbf,"init(), ");
		}
		if (instring(q->mask, FORK)) {
			strcat (xbf,q->hndlr);
			strcat (xbf,"fork(), ");
		}
		if (instring(q->mask, EXEC)) {
			strcat (xbf,q->hndlr);
			strcat (xbf,"exec(), ");
		}
		if (instring(q->mask, EXIT)) {
			strcat (xbf,q->hndlr);
			strcat (xbf,"exit(), ");
		}
		xbf[strlen(xbf)-2] = ';';

		if (instring(q->type, TTYS)) {
			strcat(xbf,"\nextern struct tty ");
			strcat(xbf,q->hndlr);
			strcat(xbf,"_tty[];\n");
		}
		xbf[strlen(xbf)-1] = NULL;
		fprintf(fdconf,"%s\n",xbf);
	}

	fprintf(fdconf,"\nstruct bdevsw bdevsw[] = {\n");
/*
 * Go through block device table and indicate addresses of required routines.
 * If a particular device is not present, fill in "nodev" entries.
 */
	for (i=0; i<=bmax; i++) {
		ptr = bdevices[i];
		fprintf(fdconf,"/*%2d*/\t",i);
		if (ptr) {
			fprintf(fdconf,"%sopen,\t%sclose,\t%sstrategy,\t%sprint,\n",
				ptr->hndlr,
				ptr->hndlr,
				ptr->hndlr,
				ptr->hndlr);
		} else {
			fprintf(fdconf,"nodev, \tnodev, \tnodev, \t0, \n");
		}
	}
	fprintf(fdconf,"};\n\n");
	fprintf(fdconf,"struct cdevsw cdevsw[] = {\n");
/*
 * Go through character device table and indicate addresses of required
 * routines, or indicate "nulldev" if routine is not present.  If a
 * particular device is not present, fill in "nodev" entries.
 */
	for (j=0; j<=cmax; j++) {
		ptr = cdevices[j];
		fprintf(fdconf,"/*%2d*/",j);
		if (ptr) {
			if (instring(ptr->mask, OPEN))
				fprintf(fdconf,"\t%sopen,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnulldev,");
			if (instring(ptr->mask, CLOSE))
				fprintf(fdconf,"\t%sclose,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnulldev,");
			if (instring(ptr->mask, READ))
				fprintf(fdconf,"\t%sread,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnodev, ");
			if (instring(ptr->mask, WRITE))
				fprintf(fdconf,"\t%swrite,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnodev, ");
			if (instring(ptr->mask, IOCTL))
				fprintf(fdconf,"\t%sioctl,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnodev, ");
			if (instring(ptr->type, TTYS))
				fprintf(fdconf,"\t%s_tty,\n",ptr->hndlr);
			else
				fprintf(fdconf,"\t0,\n");
		}
		else
			fprintf(fdconf,"\tnodev, \tnodev, \tnodev, \tnodev, \tnodev, \t0,\n");
	}
/*
 * Print out block and character device counts, root, swap, and dump device
 * information, and the swplo, and nswap values.
 */
	fprintf(fdconf,"};\n\n");
	fprintf(fdconf,"int\tbdevcnt = %d;\nint\tcdevcnt = %d;\n\n",i,j);
	fprintf(fdconf,"dev_t\trootdev = makedev(%d, %d);\n",rtmaj,rtmin);
	fprintf(fdconf,"dev_t\tpipedev = makedev(%d, %d);\n",pipmaj,pipmin);
	fprintf(fdconf,"dev_t\tdumpdev = makedev(%d, %d);\n",dmpmaj,dmpmin);
	fprintf(fdconf,"extern %sdump();\n", dmppt->handler);
	fprintf(fdconf, "int\t(*dump)() = %sdump;\n",dmppt->handler);
	fprintf(fdconf,"dev_t\tswapdev = makedev(%d, %d);\n",swpmaj,swpmin);
	fprintf(fdconf,"daddr_t\tswplo = %lu;\nint\tnswap = %d;\n\n",swplo,nswap);

/*
 * Go through the block device table, ignoring any zero entries.
 */
	if (tflag)
		printf("Block Devices\nmajor\tdevice\thandler\tcount\n");
	for (i=0; i<=bmax; i++) {
		if ((q=bdevices[i]) == 0)
			continue;
		for (counter = 0, p = table; p <= &table[tbound]; p++)
			if (equal(q->dev,p->devname))
				counter += p->mlt;

		if (!(instring(q->type, NOCNT)))
			fprintf(fdconf,"int\t%s_cnt = %d;\n",
				q->hndlr,counter);
		if (tflag)
			printf("%2d\t%s\t%s\t%2d\n",i,q->dev,q->hndlr,counter);
		q->acount = counter;
	}
/*
 * Go through the character device table, ignoring any zero entries,
 * as well as those that are not strictly character devices.
 */
	if (tflag)
		printf("Character Devices\nmajor\tdevice\thandler\tcount\n");
	for (i=0; i<=cmax; i++) {
		if ((q=cdevices[i]) == 0)
			continue;
		if (instring(q->type, CHAR) && instring(q->type, BLOCK)) {
			if (tflag)
				printf("%2d\t%s\t%s\t%2d\n",i,q->dev,q->hndlr,q->acount);
			continue;
		}
/*
 * For each of these devices, go through the configuration table and
 * look for matches. For each match, keep a count of the number of matches.
 */
		for (counter = 0, p = table; p <= &table[tbound]; p++)
			if (equal(q->dev,p->devname))
				counter += p->mlt;

		fprintf(fdconf,"\n");
		if (!(instring(q->type, NOCNT)))
			fprintf(fdconf,"int\t%s_cnt = %d;\n",
				q->hndlr,counter);
		if (tflag)
			printf("%2d\t%s\t%s\t%2d\n",i,q->dev,q->hndlr,counter);
	}

/*
 * Initialize the init, fork, exec, and exit handler buffers.
 */
	sprintf(initbf,"\nint\t(*dev_init[])() = \n{\n");
	sprintf(forkbf,"\nint\t(*dev_fork[])() = \n{\n");
	sprintf(execbf,"\nint\t(*dev_exec[])() = \n{\n");
	sprintf(exitbf,"\nint\t(*dev_exit[])() = \n{\n");
/*
 * for every entry, set up to print their init, fork, exec, or
 * exit routines, if they exist
 */
	for (q = devinfo; q <= &devinfo[dbound]; q++) {
		if (! instring(q->type, REQ)) {
			for (p = table; p <= &table[tbound]; p++) {
				if (equal(q->dev,p->devname))
					goto goon;
			}
			{
			struct t4 *tmp;
			if ((tmp = lookup(q->dev)) != 0) {
				if (atoi(tmp->value) != 0 )
					goto goon;
			}
			}
			continue;
		}
goon:
		if (instring(q->mask, INIT))
				sprintf(&initbf[strlen(initbf)],"\t%sinit,\n",q->hndlr);
		if (instring(q->mask, FORK))
				sprintf(&forkbf[strlen(forkbf)],"\t%sfork,\n",q->hndlr);
		if (instring(q->mask, EXEC))
				sprintf(&execbf[strlen(execbf)],"\t%sexec,\n",q->hndlr);
		if (instring(q->mask, EXIT))
				sprintf(&exitbf[strlen(exitbf)],"\t%sexit,\n",q->hndlr);
	
/*
 * Print any required structure definitions.
 */
		for (j=0; j<3; j++) {
			if (*(q->entries[j]) != NULL)
				fprintf(fdconf,"struct\t%s\t%s_%s[%d];\n",
					q->entries[j],q->hndlr,
					q->entries[j],counter);
		}
	}
/*
 * Write out NULL entry into init, fork, exec and exit buffers.
 * Then write the buffers out into the configuration file.
 */
	sprintf(&initbf[strlen(initbf)],"\t(int (*)())0\n};\n");
	sprintf(&forkbf[strlen(forkbf)],"\t(int (*)())0\n};\n");
	sprintf(&execbf[strlen(execbf)],"\t(int (*)())0\n};\n");
	sprintf(&exitbf[strlen(exitbf)],"\t(int (*)())0\n};\n");
	fprintf(fdconf,"%s",initbf);
	fprintf(fdconf,"%s",forkbf);
	fprintf(fdconf,"%s",execbf);
	fprintf(fdconf,"%s",exitbf);
}

max(a,b)
int a, b;
{
	return(a>b ? a:b);
}

/*
 * This routine is used to search the configuration table for some
 * specified device.  If the device is found we return a pointer to
 * that device.  If the device is not found, we search the alias
 * table for this device.  If the device is not found in the alias table
 * we return a zero.  If the device is found, we change its name to
 * the reference name of the device and re-initiate the search for this
 * new name in the configuration table.
 */

struct t *
locate(device)
char *device;
{
	register struct t *p;
	register struct t3 *r;
	for (;;) {
		for (p=table; p<= &table[tbound]; p++) {
			if (equal(device,p->devname))
				return(p);
		}
		for (r=alias; r<= &alias[abound]; r++) {
			if (equal(device,r->new)) {
				device = r->old;
				break;
			}
		}
		if (r > &alias[abound])
			return(0);
	}
}

/*
 * This routine is used to search the parameter table
 * for the keyword that was specified in the configuration.  If the
 * keyword cannot be found in this table, a value of zero is returned.
 * If the keyword is found, a pointer to that entry is returned.
 */
struct t4 *
lookup(keyword)
char *keyword;
{
	register struct t4 *kwdptr;
	for (kwdptr=parms; kwdptr<= &parms[pbound]; kwdptr++) {
		if (equal(keyword,kwdptr->indef))
			return (kwdptr);
	}
	return(0);
}

/*
 * This routine is used to map lower case alphabetics into upper case.
 */
char *
uppermap(device,caps)
char *device;
char *caps;
{
	register char *ptr;
	register char *ptr2;
	ptr2 = caps;
	for (ptr=device; *ptr!=NULL; ptr++) {
		if ('a' <= *ptr && *ptr <= 'z')
			*ptr2++ = *ptr + 'A' - 'a';
		else
			*ptr2++ = *ptr;
	}
	*ptr2 = NULL;
	return (caps);
}

/*
 * This routine enters the device in the configuration table.
 */
enterdev(vec,ml,name)
int	vec, ml;
char	*name;
{
	tbound++;
	if (tbound == TSIZE) {
		printf("Configuration table overflow\n");
		exit(1);
	}
/*
 * Write upper case define statement and sequence number for device.
 */
	fprintf(confhp,"#define\t%s_%d %d\n",
		uppermap (name,capitals),
		q->dcount, ml);
	setq();
	p->devname = q->dev;
	p->vector = (vec == 0) ? q->vect : vec;
	p->type1 = q->type;
	p->handler = q->hndlr;
	p->count = q->dcount;
	p->mlt = ml;
	p->blk = q->block;
	p->chr = q->charr;
	p++;
	q->dcount++;
}
