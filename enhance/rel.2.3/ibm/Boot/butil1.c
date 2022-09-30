/*   @(#)butil1.c	2.3 - 5/15/87 */
/*	sccsid @(#)butil1.c	1.7	*/

/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * butil1.c
 *
 *	utility routines to support bload.c
 *
 * Modification History:
 *
 * M000	uport!dwight Tue Feb 11 15:23:55 PST 1986
 *	Modified to run on the IBM AT.
 * M001 uport!larry  Wed Mar 12 1986
 *  	Added bad-track mapping
 * M002	uport!rex	Sun May 17 16:39:15 PDT 1987
 *	Added dosboot() routine to look for dos partition, read it
 *	in and call bootit() to boot DOS.
 * M003	uport!rex	Thu May 28 00:32:20 PDT 1987
 *	Changed fsdelta to a long and added some (long) casts to
 *	expressions using fsdelta to fix problem of booting from partitions
 *	that are offset greater than 32Mb's into the disk
 */

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/inode.h"
#include "sys/ino.h"
#include "sys/fblk.h"
#include "sys/dir.h"

#define ltoa(x)	( (x) << 12 )
#define SEC_SIZE 512

long breadi() ;
char	gbuf[BSIZE];			/* generic buffer */

#ifdef	IBMAT				/* M000	*/
long	gbuf_cache;			/* M000: current block */
#else	IBMAT				/* M000	*/
long	gbuf_cache = -1;		/* current block */
#endif	IBMAT				/* M000	*/
struct	inode	in;			/* use static version */

/*
	space for holding block pointers from i-node
*/

/* first a block for doubly indirect */

long indnum ;
long indaddr[  NINDIR ] ;

/* space for direct-pointers & 1 indirect block plus lots of others */

long	iaddr[NADDR-3+NINDIR];


char	*getcomp() ;
/*
 * bnami
 *	Scan a path, starting at given inode.
 *
 * Returns with value = inum (0 if not found)
 */

short
bnami(inum, path)
short		inum;
register char	*path;
{
	struct direct	comp, dir;
	register	i;
	long		count;
	long		offset;

	/*
	 * Loop, scanning path.
	 */

	for(;;) {
		while(*path == '/')			/* skip leading /'s */
			path++;

		/*
		 * If null path, found it!
		 */

		if (*path == '\0')
			return(inum);

		/*
		 * Get inode, find entry in directory.
		 * It must be a directory.
		 */

		if (biget(inum) == 0)
			return(0);		/* didn't find */

		if ((in.i_mode & IFMT) != IFDIR) {
			return(0);		/* not a directory */
		}

		/*
		 * Loop thru directory, looking for name.
		 */
		
		path = getcomp(path, comp.d_name);	/* get component */

		offset = 0;

		while(count=breadi(offset, &dir , getDS() ,(long)sizeof(dir))) {
			if (dir.d_ino != 0) {
				for(i = 0; i < DIRSIZ; i++)
					if (lower(comp.d_name[i]) 
						!= lower(dir.d_name[i]))
						goto nextent;
				goto nextdir;
			}
		nextent:
			offset += count;
		}
		return(0);			/* ran out of directory */
	nextdir:
		inum = dir.d_ino;		/* this file */
	}
}

/*
 * breadi
 *	"Read" a unix file.
 *
 */

long breadi(loc, addr , count)
long		loc;
long		addr;
long		count;
{
	unsigned	xc;			/* local transfer count */
	long		tcount;			/* total transfer count */
	unsigned	offset;			/* offset in gbuf */

	/*
	 * Restrict count, if it would go beyond EOF.
	 */

	if (loc + count > in.i_size)
		count = in.i_size - loc;


	tcount = 0;
	while(count != 0) {
		offset = loc % BSIZE;
		xc = BSIZE - offset;
		if (xc > count)
			xc = count;

		dread(iaddr[(int)(loc/BSIZE)]);

		iomove(&gbuf[offset], addr , xc);

		loc += xc;
		tcount += xc;
		count -= xc;

		/* increment addr by xc - but
		   don't forget this is an address */

		addr=((addr>>12)&0x000ffff0L) + (addr&0x0000ffffL) + (long)xc ;
		addr=((addr&0xfffffff0L)<<12) + (addr&0x0000000fL) ;

	}
	return(tcount);
}

/*
 * biget
 *	input disk-version of inode.
 *
 */
 
short
biget(inum)
short	inum;
{
	char		*p1, *p2;
	struct dinode	*dp;
	register int	i;
	int		readat ,
			readfrom ;

	if (inum == 0)
		return(0);			/* Sorry! */

	dread( itod(inum) );
	dp = (struct dinode *)gbuf;
	dp += ( itoo(inum) );

	in.i_mode = dp->di_mode;
	in.i_size = dp->di_size;

	/*
	 * Get address pointers.
	 * Our long words are held low[0],low[1],high[0],high[1]
	 * The 3 bytes in the dinode are low[0],low[1],high[0].
	 */

	p1 = (char *) iaddr ;
	p2 = (char *) dp->di_addr;

	for(i = 0; i < NADDR; i++) {
		*p1++ = *p2++ ;
		*p1++ = *p2++ ;
		*p1++ = *p2++ ;
		*p1++ = 0 ;
	}

	/*
	 *	save second indirect #
	 */

	indnum = iaddr[NADDR-2] ;

	/*
	 * If file is "long", get 1st indir-block.
	 */


	if (iaddr[NADDR-3] != 0) {
		dread(iaddr[NADDR-3]);
		iomove(gbuf, &iaddr[NADDR-3] , getDS() , BSIZE);
	}

	/*
	 *	if file is "very long" get 2nd indirect block
	 */

	if (indnum != 0)
	{
		dread(indnum);
		iomove(gbuf, indaddr , getDS() , BSIZE ) ;


		readat = NADDR - 3 + NINDIR ;
		readfrom = 0 ;

		while ( indaddr[readfrom] != 0 )
		{
			dread(indaddr[readfrom++]);
			iomove(gbuf,&iaddr[readat],getDS(),BSIZE);
			readat += NINDIR ;
		}
	}

	return(inum);				/* got it */
}

/*
 * dread
 *	Read the xfs disk.  Transfers BSIZE bytes into "gbuf" (static).
 *
 * If block is already cached there, done.
 */
extern	long	fsdelta;	/* sector offset to file-system M003 */

dread(bno)
long	bno;
{
	register int	offset;
	long	secno;

	if (bno == gbuf_cache)
		return;

	secno = ( bno * ( BSIZE/SEC_SIZE) )  + fsdelta;
	for(offset = 0; offset < BSIZE; offset += SEC_SIZE)
		kdriver(secno++, &gbuf[offset], getDS());


	gbuf_cache = bno;
}

kdriver( sec , w , x )
long sec ;
{
	unsigned int cylinder ;
	unsigned int sector ;
	unsigned int head ;
	unsigned int track ;
	unsigned int tracks_per_cylinder ;
	unsigned int fiddle ;		/*
					 * fiddle is required as Intel number
					 * sectors from 0 for hard discs
					 * and from 1 for floppies
					 */
	extern struct {
			unsigned int	number_of_cylinders ;
			unsigned char	number_of_fixed_heads ;
			unsigned char	number_of_removable_heads ;
			unsigned char	number_of_sectors_per_track ;
		} boot_db ;

	/*
	 *	if # removable discs is not zero then we have a floppy
	 */

	if( boot_db.number_of_removable_heads )
	{
		tracks_per_cylinder = boot_db.number_of_removable_heads ;
		fiddle = 1 ;
	}
	else
	{
		tracks_per_cylinder = boot_db.number_of_fixed_heads ;

/* M000: Intel may number their hard disk sectors starting at 0,
 *	but IBM sure doesn't.
 */
#ifdef	HARDDISK					/* M000 */
		fiddle = 1 ;				/* M000 */
#else	HARDDISK					/* M000 */
		fiddle = 0 ;
#endif	HARDDISK					/* M000 */
	}

	sector = ( sec % boot_db.number_of_sectors_per_track ) + fiddle ;
	track = sec / boot_db.number_of_sectors_per_track ;

	cylinder = track / tracks_per_cylinder ;
	head = track % tracks_per_cylinder ;

#ifdef	IBMAT						/* Start M000 */
#ifdef HARDDISK
map_bad_track(&cylinder,&head);				/* M001 */
#endif HARDDISK
/* The following code is required to insure that the high two bits
 * of the cylinder # are passed along in "sector".
 */
	sector &= 0x003F;				/* clear dest. bits */
	sector |= ((cylinder & 0x0300) >> 2);
	driver (sector, head, cylinder , w , x );
#else	IBMAT						/* M000 */
	driver ( sector | (head<<8) , cylinder , w , x ) ;
#endif	IBMAT						/* End M000 */
}


/*
 * getcomp
 *	Get next path component from a string.
 *
 * Return next position in string as value.
 */

char *
getcomp(path, comp)
register char *path;
register char *comp;
{
	register i;
	char	c;

	for(i = 0; *path && *path != '/'; i++) {
		c = *path++;
		if (i < DIRSIZ)
			*comp++ = c;
	}
	while(i++ < DIRSIZ)
		*comp++ = 0;

	return(path);
}

/*
 * lower
 *	Lower-case a character.
 */

lower(c)
register c;
{
	if (c >= 'A' && c <= 'Z')
		return	(c + ('a'-'A')) ;
	else
		return  (c) ;
}

#ifdef	IBMAT				/* start M002 */
#ifdef	HARDDISK
#ifdef	DOSBOOT

struct p_entry {
	unsigned char	b_ind;
	unsigned char	b_head;
	unsigned char	b_sec;
	unsigned char	b_cyl;
	unsigned char	s_ind;
	unsigned char	s_head;
	unsigned char	s_sec;
	unsigned char	s_cyl;
	long		p_offset;
	long		p_size;
} dosentry;
struct p_entry  *pt;

/*
 *  Read the partition table from 1st sector of fixed disk, check for
 *  a DOS system indicator, read the DOS partition boot sector into
 *  a buffer and if valid, pass the buffer and the table entry to bootit()
 */
dosboot()
{
	register	i;

	gbuf_cache = -1;
	driver(1, 0, 0 , gbuf, getDS());	/* (S,H,C,off,seg) */
	if (*((int *) &gbuf[0x1FE]) != 0xAA55)	/* Valid Master Boot? */
		return (1);
	pt = (struct p_entry *) &gbuf[0x1BE];
	for (i=0; i<4; ++i, ++pt)
		if (pt->s_ind == 1 || pt->s_ind == 4)	/* DOS indicators */
			break;
	if (i >= 4)
		return(2);
	iomove(pt, &dosentry, getDS(), 16);	/* save the partition entry */
	driver(pt->b_sec, pt->b_head, pt->b_cyl, gbuf, getDS());
	if (*((int *) &gbuf[0x1FE]) != 0xAA55)	/* Valid boot sector? */
		return (3);
	dosentry.b_ind = 0x80;
	bootit(gbuf, getDS(), dosentry);
}
#endif	DOSBOOT
#endif	HARDDISK
#endif	IBMAT				/* end M002 */
