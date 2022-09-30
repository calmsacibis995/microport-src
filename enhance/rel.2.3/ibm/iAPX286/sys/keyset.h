/* uportid = "@(#)keyset.h	Microport Rev Id  1.3.3 6/18/86" */
/*
 * This file describes the console driver setkey ioctl
 */

#ifdef	NCRKEY
#define	NUMFKEYS	12
#else	NCRKEY
#define	NUMFKEYS	10
#endif	NCRKEY

#define	FKEYLEN		16

struct keyset {
	int		ks_fkey;				/* Number of function key */
	int		ks_len;					/* Length of string */
	char	ks_str[ FKEYLEN ];		/* String to be generated */
};

#define	IOCKEYSET	('k'<<8 | 0)
#define	IOCKEYGET	('k'<<8 | 1)
