/* uportid = "@(#)format.h	Microport Rev Id  1.3.4 6/23/86" */
/* 
 * The following definitions are straight out of 215.h. Their only
 * purpose is to allow the 'format' command to work when used on 
 * the floppy device.
 */
struct	i215format {
	char	f_trtype;		/* format track-type code */
	char	f_pattern0;		/* pattern; depends on f_trtype */
	char	f_pattern1;
	char	f_pattern2;
	char	f_pattern3;
	char	f_interleave;		/* interleave-factor */
};

struct	i215ftk	{
	int	f_cyl;			/* cylinder # */
	int	f_track;		/* track # */
	int	f_intl;			/* interleave factor */
	int	f_skew;			/* track skew -- ignored by 215 */
	char	f_type;			/* format type-code */
	char	f_pat[4];		/* pattern data */
};

/*
 * iSBC 215 ioctl mnemonics.
 */

#define	I215_IOC_FMT		(('W'<<8)|0)
#define I215_FLOPPY		(('W'<<8)|1)
#define I215_CHAR		(('W'<<8)|2)
#define I215_NTRACK		(('W'<<8)|3)
#define FORMAT_DATA		(('W'<<8)|4)
#define I215_CTEST		(('W'<<8)|5)
