static char *uportid = "@(#)malloc.c	Microport Rev Id 1.3.3  6/18/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)malloc.c	1.6 */
#include "sys/param.h"
#include "sys/types.h"
#ifdef ATMERGE
#include "sys/sysmacros.h"
#include "sys/realmode.h"
#endif /* ATMERGE */
#include "sys/systm.h"
#include "sys/map.h"


#ifdef ATMERGE

/*
 * dossize holds the current click size of allocated DOS memory,
 * which is the same as the current lowest click address not in
 * use by DOS.
 */
extern unsigned int dossize;
int topalloc = 0;

/*
 * Allocate 'size' units from the given map.
 * Return the base of the allocated space.
 * In a map, the addresses are increasing and the
 * list is terminated by a 0 size.
 * The swap map unit is 512 bytes.
 *
 * Algorithm is first-fit for all but coremap,
 * which is last-fit to maximize the chance
 * that DOS memory will be free.  This cuts
 * down on swapping when a DOS program
 * is executed.  When allocating from
 * coremap we normally allocate from
 * the beginning of the last free slot rather
 * than the end to allow incore expands to
 * go quickly.  However, if topalloc is set
 * then we allocate from the top of the last
 * slot.  This is necessary when allocating
 * memory which will be locked so that we
 * don't end up locking something into DOS
 * memory.
 *
 * The only time we can find a piece of memory
 * under dossize is when someone is in the
 * process of trying to allocate DOS memory,
 * has set dossize, but has not yet allocated
 * the memory.
 */

unsigned int malloc(mp, size)
struct map *mp;
{
	register unsigned int a;
	register struct map *bp;

	if (mp == coremap) {
		/* last fit */
		for (bp = mapstart(mp); bp->m_size; bp++)
			;
		while (--bp >= mapstart(mp)) {
			if (bp->m_size >= size) {
				if (topalloc) {
					a = bp->m_addr + bp->m_size - size;
				} else if (bp->m_addr < dossize) {
					a = bp->m_addr + bp->m_size - size;
					if (a < dossize) continue;
				} else {
					a = bp->m_addr;
					bp->m_addr += size;
				}
				if ((bp->m_size -= size) == 0) {
					do {
						bp++;
						(bp-1)->m_addr = bp->m_addr;
					} while ((bp-1)->m_size = bp->m_size);
					mapsize(mp)++;
				}
				return(a);
			}
		}
	} else {
		/* first fit */
		for (bp = mapstart(mp); bp->m_size; bp++) {
			if (bp->m_size >= size) {
				a = bp->m_addr;
				bp->m_addr += size;
				if ((bp->m_size -= size) == 0) {
					do {
						bp++;
						(bp-1)->m_addr = bp->m_addr;
					} while ((bp-1)->m_size = bp->m_size);
					mapsize(mp)++;
				}
				return(a);
			}
		}
	}
	/* no fit */
	return(0);
}

/*
 * Allocate the requested space for a DOS process.
 * DOS space must always start at location zero.
 * Click 0 is always allocated to DOS and is
 * never in the coremap (because malloc returns
 * 0 to mean error).  Therefore we ask for memory
 * starting at click 1, and we subtract 1 from the
 * size we need.
 */
unsigned int
dosmalloc(size)
unsigned int size;
{
	register struct map *bp;

	if (dossize!=1)
		dosfree();   /* if a realloc, free before allocing new size */
	dossize = size;	/* number of first click we don't need */
	size--;		/* since click 0 is always available   */

	bp = mapstart(coremap);
	if ((bp->m_addr != 1) || (bp->m_size < size)) {
		memshuffle();
		if (bp->m_addr != 1)
			return (dossize=1);
		if (bp->m_size < size) {
			size = bp->m_size;	/* take what we can get */
			dossize = size+1;	/* record what we got */
		}
	}

	bp->m_addr += size;
	if ((bp->m_size -= size) == 0) {
		do {
			bp++;
			(bp-1)->m_addr = bp->m_addr;
		} while ((bp-1)->m_size = bp->m_size);
		mapsize(coremap)++;    
	}

	return(++size);
}


/*
 * Free the previously allocated DOS space
 * DOS space must always start at location zero.
 * Click 0 is always allocated to DOS and is
 * never in the coremap (because malloc returns
 * 0 to mean error).  Therefore we release memory
 * starting at click 1, and we subtract 1 from the
 * size we have.
 */
dosfree()
{
	extern unsigned int altstart, altseparate, screenbytes;

	if (dossize>1) {
		mfree(coremap, dossize-1, 1);
		dossize = 1;
		mfree(coremap,btoc(screenbytes),altstart);
	}
}

#else	/* -ATMERGE */

/*
 * Allocate 'size' units from the given map.
 * Return the base of the allocated space.
 * In a map, the addresses are increasing and the
 * list is terminated by a 0 size.
 * The swap map unit is 512 bytes.
 * Algorithm is first-fit.
 */
unsigned int malloc(mp, size)
struct map *mp;
{
	register unsigned int a;
	register struct map *bp;

	for (bp = mapstart(mp); bp->m_size; bp++) {
		if (bp->m_size >= size) {
			a = bp->m_addr;
			bp->m_addr += size;
			if ((bp->m_size -= size) == 0) {
				do {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
				} while ((bp-1)->m_size = bp->m_size);
				mapsize(mp)++;
			}
			return(a);
		}
	}
	return(0);
}
#endif /* ATMERGE */

/*
 * Free the previously allocated space aa
 * of size units into the specified map.
 * Sort aa into map and combine on
 * one or both ends if possible.
 */
mfree(mp, size, a)
struct map *mp;
register unsigned int a;
{
	register struct map *bp;
	register unsigned int t;

	bp = mapstart(mp);
	for (; bp->m_addr<=a && bp->m_size!=0; bp++);
	if (bp>mapstart(mp) && (bp-1)->m_addr+(bp-1)->m_size == a) {
		(bp-1)->m_size += size;
		if (a+size == bp->m_addr) {
			(bp-1)->m_size += bp->m_size;
			while (bp->m_size) {
				bp++;
				(bp-1)->m_addr = bp->m_addr;
				(bp-1)->m_size = bp->m_size;
			}
			mapsize(mp)++;
		}
	} else {
		if (a+size == bp->m_addr && bp->m_size) {
			bp->m_addr -= size;
			bp->m_size += size;
		} else if (size) {
			if (mapsize(mp) == 0) {
				printf("\nDANGER: mfree map overflow %x\n", mp);
				printf("  lost %d items at %d\n", size, a);
				return;
			}
			do {
				t = bp->m_addr;
				bp->m_addr = a;
				a = t;
				t = bp->m_size;
				bp->m_size = size;
				bp++;
			} while (size = t);
			mapsize(mp)--;
		}
	}
	if (mapwant(mp)) {
		mapwant(mp) = 0;
		wakeup((caddr_t)mp);
	}
}
