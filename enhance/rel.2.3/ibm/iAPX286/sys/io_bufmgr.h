/* @(#)io_bufmgr.h	1.1 */

/* routines defined in io_bufmgr.c */
char *	io_getbuf  ();		/* returns kernel virtual address */
int	io_expbuf  ();		/* returns offset of new memory */
char *	io_mapbuff ();		/* returns kernel virtual address */
void	io_freebuf ();
int	io_getsel  ();		/* returns selector obtained */
void	io_freesel ();

/* io_bufmgr macros */

/* translate kernel virtual buffer address to global selector */
#define	io_buftosel(adrs) ((unsigned long) adrs >> 19)

