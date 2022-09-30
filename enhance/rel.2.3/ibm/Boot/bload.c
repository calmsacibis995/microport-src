/*   @(#)bload.c	2.3 - 4/28/87 */
/*	sccsid @(#)bload.c	1.8	*/

/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* Modification History
 *
 * M000:	uport!dwight Tue Feb 11 14:50:58 PST 1986
 *	Upgraded to run on the IBM AT. 
 * M001:	uport!dwight Sun Mar 9 1986
 *	Now uses values from littleboot for #heads, et al
 *	Changed search path to find "/system5".
 * M002		uport!larry  Tue  Feb 11 1986
 * 	Badtrack mapping added
 * M003		uport!rex	4/28/87
 *	Modified to ask for a boot pathname, or timeout after
 *		one minute and use the file name passed to bload()
 *	Modified to recognize the boot name "dos" and try to find and
 *		boot a DOS partition	( ifdef'ed with DOSBOOT )
 *	Modified to pass a block of info to the kernel in low memory
 * M004	uport!rex	Thu May 28 00:32:20 PDT 1987
 *	Changed fsdelta to a long and added some (long) casts to
 *	expressions using fsdelta to fix problem of booting from partitions
 *	that are offset greater than 32Mb's into the disk
 */

/*
 *
 *		b l o a d . c
 *		~~~~~~~~~~~~~
 *
 *	Bootstrap loader
 *
 *	This module loads an STL format file from Unix filestore . 
 *	It is intended for loading Unix itself but other development
 *	work may be achieved by using this program to boot in test files.
 *
 *	Expected object format is STL
 *
 */

#include <aouthdr.h>
#include <filehdr.h>
#include <scnhdr.h>

#define	ROOTINO		0x2
#define KERNEL_BASE	0x1000L		/* Start M001 */
#define WAKE_UP_SIZE	0x6L

#define TEXT_INFO 0x9a000000L
#define DATA_INFO 0x92000000L

#define ALIGN_SEG(x)	( (x+15) & 0xfffffff0L )
#define ltoa(x)		( (x) << 12 )
	
/* 
 * define structure that includes both main header and extra header so
 * entire header information for file may be read at once.
 * Also declare buffer for holding transfers.
 *
 */

struct headers {
	struct filehdr mainhdr ;
	struct aouthdr secondhdr ;
}head ;

SCNHDR  section;

#ifndef	IBMAT				/* M000	*/
/*
 * default name of file to "boot"
 */

char *defname = "/unix" ;
#endif	IBMAT				/* M000	*/

long getlong() ;

/*
 *
 *	bload
 *	~~~~~
 *
 *	Called from boot.a86 ( startoff ) to load the image.
 *
 *	Loads an STL format file. 
 *
 *	parameter - path - pointer to path name of file to load	
 *
 */

#ifdef	IBMAT						/* M000	*/
extern long gbuf_cache;					/* M000 */
extern unsigned int cylsec;				/* M000 */
extern unsigned int maxcylsec, headdrive;		/* M001 */
extern unsigned char pehd,pecy,pesc;			/* M002 */
unsigned int pbcyl, pbsec, secpcyl;			/* M000 */
extern long fsdelta;					/* M004 */

struct {
	unsigned int number_of_cylinders;		/* M000 */
	unsigned char number_of_fixed_heads;		/* M000 */
	unsigned char number_of_removable_heads;	/* M000 */
	unsigned char number_of_sectors_per_track;	/* M000 */
} boot_db;						/* M000 */

struct  bad_track_map   {
        unsigned int    bad_cylinder;
        unsigned char   bad_track;
        unsigned int    new_cylinder;
        unsigned char   new_track;
};

union {							/* M002 */
	unsigned char pebuf[512];			/* M002 */
	struct bad_track_map badt[64];			/* M002 */
}btbuf ;						/* M002 */

char	*savpath;					/* M003 */
int	autoflag;					/* M003 */
#ifdef DOSBOOT
char	dosstr[4];					/* M003 */
#endif DOSBOOT

#endif	IBMAT

bload ( path )
char *path ;
{

/* Start M000: Initialize variables that were previous initialized data	*/

#ifdef	IBMAT
	int	i;					/* M003 */
	char	*getbootnam();				/* M003 */

#ifdef	LOWDENSITY
	boot_db.number_of_cylinders = 40;
	boot_db.number_of_fixed_heads = 0;
	boot_db.number_of_removable_heads = 2;
	boot_db.number_of_sectors_per_track = 9;
#endif	LOWDENSITY
#ifdef	HIDENSITY
	boot_db.number_of_cylinders = 80;
	boot_db.number_of_fixed_heads = 0;
	boot_db.number_of_removable_heads = 2;
	boot_db.number_of_sectors_per_track = 15;
#endif	HIDENSITY
#ifdef	HARDDISK					/* Start M001	*/
	switch ((unsigned char) headdrive & 0x00FF) {
		case 0x80:
			boot_db.number_of_fixed_heads = 
					((headdrive & 0xFF00) >> 8) + 1;
			boot_db.number_of_removable_heads = 0;
			boot_db.number_of_cylinders = 0; /* Not used */
			break;
		default:
			break;
	}

	boot_db.number_of_sectors_per_track = (maxcylsec & 0x003F);

/* set up the beginning partition cylinder number in pbcyl.
 * don't forget the extra two bits in the high byte of the 
 * "sec" part of cylsec.
 */
	pbcyl = (unsigned int) ((cylsec & 0xFF00) >> 8);
	pbcyl += (unsigned int) ((cylsec & 0x00C0) << 2);
	pbsec = (unsigned int) cylsec & 0x003F;
	secpcyl = boot_db.number_of_fixed_heads *
			boot_db.number_of_sectors_per_track;
	fsdelta = (long) (pbcyl * secpcyl) + (long) pbsec - 1; /* M004 */
/*						begin		* M002 */
/* read in badtrack table if valid */
	driver(pesc, pehd, pecy, btbuf.pebuf, getDS()); /* part end rec */
	if (*((int *) &btbuf.pebuf[0x1FE]) == 0xAA55)
	{
		int sector ; /*temp*/
		sector = (pesc & 0x3f)-1 ; /* previous sector */
		sector |= pesc & 0xC0;
		driver(sector,pehd,pecy,btbuf.pebuf,getDS()); /* is bad track table */
	} else {

	/* if no valid badtrax , initialize table */
		int i;
		for (i=0; i>=64 ;i++)
			btbuf.badt[i].bad_cylinder = 0xffff;
	};
/*						end		* M002 */


#endif	HARDDISK
	
	gbuf_cache = -1;		/* avoid initialized data	*/

	/* path is now passed already initialized;	End M003	*/

#endif	IBMAT				/* End M000			*/


#ifndef	IBMAT				/* M000 			*/
	if ( *path == '\0' )
		path =  defname ;
#else	/* is IBMAT */			/* M003	START			*/

/*
 * get name of file to boot
 */
	putchar(' ');
#ifdef DOSBOOT
	dosstr[0] =  'd';
	dosstr[1] =  'o';
	dosstr[2] =  's';
	dosstr[3] = '\0';
#endif DOSBOOT
	savpath = path;
	autoflag = 1;
	do {
getpath:
		path = getbootnam(savpath, autoflag);
		autoflag = 0;
#ifdef DOSBOOT
		for (i=0; i < 4; ++i)
			if (dosstr[i] != path[i])
				break;
		if (i == 4)
			if ( dosboot() )
				goto getpath;
#endif DOSBOOT
	} while ( biget(bnami(ROOTINO, path)) == 0 );

#endif	/* if IBMAT	*/		/* M003 END	*/
#ifndef	IBMAT				/* M003 	*/

	if ( biget(bnami(ROOTINO, path)) == 0 )
		halt() ;

#endif	/* if not IBMAT	*/		/* M003 	*/

	/*
	 * Get header info from file
	 */

	breadi(0L, (char *)&head, getDS(), (long)sizeof head);

	/*
	 * check magic number etc.
	 */

	if ( head.mainhdr.f_magic != I286LMAGIC )
		halt() ;

	/*
	 * perform the actual load
	 */

	loadstl();

	/* never returns */

}

/*
 *
 *	loadstl
 *	~~~~~~~
 *
 *	The boot for the kernel is rather special.
 *      The file contains the first part of the system GDT.
 *
 *	The layout of memory for the kernel is :-
 *
 *	-----------------------------------------	0x000000
 *	|	interrupt table			|
 *	|---------------------------------------|	0x001000
 *	|	wake-up block			|
 *	|---------------------------------------|	+ 6?
 *	|	G.D.T.				|
 *	|---------------------------------------|
 *	|	rest of				|
 *	|	kernel data (initialised)	|
 *	| 	(may be more than 1 seg.)	|
 *	|---------------------------------------|
 *	|	kernel bss			|
 *	|	(may be more than 1 seg.)	|
 *	-----------------------------------------
 *	|	kernel text			|
 *	|	(may be more than 1 segment)	|
 *	|---------------------------------------|
 *
 *
 *	This gets loaded as follows:-
 *
 *	1) the data area is copied from file into memory
 *
 *	2) the bss area is initialised to zero
 *
 *	3) the text area is copied from file into memory
 *
 *      4) the section headers are stepped through to fill in the
 *         physical addresses in the entries in the GDT, other
 *         properties, limits, etc.
 *
 *
 *
 */


loadstl ()
{
	long    segnum;
	long    vsize;
	long    physaddr;
	long    flags;

	long	data_offset ,
		data_start ,
		data_size ,
		text_offset ,
		text_start ,
		text_size ,
		bss_start ,
		bss_size ,
		shdr_offset ,
		no_of_nscns     ;

	/*
	 * 	set known size and offset information
	 *
	 *	one strange thing here is that the first bytes of
	 *	data are ignored - this is where the wake-up block
	 *      is going to be and where the boot PROM also has
	 *	access the disc
	 */

	/*
	 *	first the sizes of the various areas
	 */

	data_size = head.secondhdr.dsize;
	text_size = head.secondhdr.tsize;
	bss_size  = head.secondhdr.bsize;

	/*
	 *	offsets within file of text and data
	 */

	text_offset = sizeof(head) + head.mainhdr.f_nscns * sizeof(SCNHDR);
	data_offset = text_offset + text_size ;
	shdr_offset = sizeof(head);

	/*
	 *	start addresses in memory of the areas
	 */

	data_start = ALIGN_SEG ( KERNEL_BASE ) ;
	bss_start  = ALIGN_SEG ( data_start + data_size ) ;
	text_start = ALIGN_SEG ( bss_start  + bss_size ) ;

	/*
	 *	load data image
	 */

	breadi ( data_offset + WAKE_UP_SIZE ,
	  ltoa ( data_start ) + WAKE_UP_SIZE , data_size - WAKE_UP_SIZE ) ;

	/*
	 *	zeroise bss
	 */

	zeroise ( ltoa ( bss_start ) , ALIGN_SEG(bss_size) ) ;

	/*
	 *	load text image - but to fixed address away from
	 *	area used by PROM for system tables
	 */

	breadi ( text_offset , ltoa ( text_start ) , text_size ) ;

	/*
	 *      fill in the GDT from the section headers
	 *
	 *
	 *      read in the section hdrs - one at a time (buffered in butil)
	 *      select ones we're interested in BSS, DATA and TEXT
	 *	and fill in the appropriate entry in the GDT
	 *
	 */

	no_of_nscns = head.mainhdr.f_nscns;

	while ( no_of_nscns--) {
		breadi(shdr_offset, (char *)&section
				  , getDS(), (long)SCNHSZ);
		shdr_offset += SCNHSZ;

		/*
		 *      setup the GDT entry for this fst entry
		 *      content limit points to the highest
		 *      addressable byte so is 1 less than the length
		 *      (size) of the area - this means a limit of 0
		 *      is in fact a segment containing 1 byte.
		 *
		 */

		segnum = section.s_vaddr >> 19;
		vsize = section.s_size;
		if (section.s_flags & STYP_TEXT)
		{       flags = TEXT_INFO;
			physaddr = text_start;
			text_start += vsize ;
		}
		else if (section.s_flags & STYP_DATA)
		{       flags = DATA_INFO;
			physaddr = data_start ;
			data_start += vsize ;
		}
		else if (section.s_flags & STYP_BSS)
		{       flags = DATA_INFO;
			physaddr = bss_start ;
			bss_start += vsize ;
		}
		else
			continue;
		setgdt( (segnum * 8) + WAKE_UP_SIZE + ltoa(KERNEL_BASE)
			, (short)(vsize - 1), physaddr | flags);
	}
	/*
	 *	start the loaded object
	 *
	 *	Need to work out start address
	 *
	 *	Only problem is we are in real address mode
	 *	and selector is for protected - so need to read GDT
	 *	entry corresponding to selector to get a real address
	 *	for the long jump
	 *
	 */


	text_start = getlong( ltoa(KERNEL_BASE)
			    + WAKE_UP_SIZE
			    + 2
			    + ((head.secondhdr.entry>>16) & 0x0000fff8L) );

	text_start &= 0x00ffffffL;      /* top byte is access rights */

	text_start += head.secondhdr.entry & 0x0000ffffL;

	ljump( ltoa (text_start & 0xfffffff0L) + ( text_start & 0x0000000fL));
}

#ifdef	IBMAT						/* Start M002	*/
/* routine to search for and replace bad cylinders and heads */
map_bad_track(cyl,head)
int *cyl,*head;
{
	int i;
	for (i=0;i <64 ;i++)
		if (btbuf.badt[i].bad_cylinder > *cyl ) return;
		else if ((btbuf.badt[i].bad_cylinder == *cyl) &&
			 (btbuf.badt[i].bad_track == *head))
			  {
				*cyl = btbuf.badt[i].new_cylinder;
				*head = btbuf.badt[i].new_track;
			  };
}
#endif	IBMAT						/* End M002	*/

#ifdef	IBMAT						/* start M003	*/
char	newpath[30];
/*
 * get name of file to boot
 * if autoboot is set wait one minute and return default name (defpath)
 */
char *
getbootnam(defpath, autoboot)
	char	*defpath;
	int	autoboot;
{
	int	i, getname;
	char	*retpath;
	char	c;

	if (autoboot) {
		pit_init();	/* initialize tenmicrosec() */
		i = 20000;	/* 6000 */
		while (--i && !ischar())
			tenmicrosec();
		if (ischar())
			getname = 1;
		else
			getname = 0;
	} else {
		putchar('b');
		putchar('o');
		putchar('o');
		putchar('t');
		putchar(':');
		putchar(' ');
		getname = 1;
	}
	i = 0;
	if (getname) {
		while((c = getchar()) != '\r') {
			putchar(c);
			if (c == '\b') {
				if (i) {
					putchar(' ');
					putchar('\b');
					i--;
				} else
					putchar(' ');
				continue;
			}
			newpath[i++] = c; 
		}
		newpath[i] = 0;
	}
	if (i == 0) {
		retpath = defpath;
		while ( defpath[i] )
			putchar( defpath[i++] );
	} else
		retpath = newpath;
	putchar('\r');
	putchar('\n');
	return retpath;
}
#endif /* IBMAT						end M003	*/
