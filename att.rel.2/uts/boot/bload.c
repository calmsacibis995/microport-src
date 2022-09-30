/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)bload.c	1.7 - 85/08/12 */

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
#define KERNEL_BASE	0x1000L
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

/*
 * default name of file to "boot"
 */

char *defname = "/unix" ;

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

bload ( path )
char *path ;
{
	if ( *path == '\0' )
		path =  defname ;

	if ( biget(bnami(ROOTINO, path)) == 0 )
		halt() ;

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
