/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
 * @(#)wn.h	1.14
 *	215/218 Driver declarations. Support for 215A/B/G
 *
 * Details of the 215/218 hardware may be found in Intel manual order
 * number 121593-002 (215), 121583-001 (218).
 */

#define	SPL		 spl6		/* for driver mutex */
#define	I215RETRY	 4		/* retry count */
#define	NUMSPINDLE	 8		/* # spindles per board */
#define	FIRSTREMOV	 4		/* first removable unit-number */
#define NEXT_REMOVE_UNIT 4		/* increment to next removable unit */
#define	FIXEDMASK	 (FIRSTREMOV-1)	/* mask for fixed-unit given unit */
#define UNIT_MASK	 0x30		/* unit field of cib status byte */

/*
 * The minor device number is used as index into the i215minor array.
 * This array is defined in wn.conf.c. The defines below are to help
 * understand the encoding, but it is still confusing.
 */
#define BOARD(dev)	((i215minor[minor(dev)]>>12)&0xf)
#define	UNIT(dev)	((i215minor[minor(dev)]>>8)&0xf)
#define	DRTAB(dev)	((i215minor[minor(dev)]>>4)&0xf)
#define SLICE(dev)	((i215minor[minor(dev)])&0xf)

#define PHYSADDR( X ) (a20bit ? ADDR86( physaddr( X ) ) : physaddr( X ) ); /* get physical addres for proper board type */

#define i215MINOR(board,unit,drtab,slice) \
	((board<<12)|(unit<<8)|(drtab<<4)|slice)

/* makes an 8086 address */
#define ADDR86(X)	((X&0xffffl)|(((X)<<12)&0xf0000000l))

/*
 * Slice structure.  One per drtab[] entry.
 */

struct	i215slice {
	daddr_t	p_fsec;			/* first sector */
	daddr_t	p_nsec;			/* number sectors */
};

/*
 * Per-board configuration.  One of these per 215/218.
 */

struct	i215cfg	{
	long		c_wua;			/* Physical Wake-Up Address */
	char		c_devcod[(NUMSPINDLE/FIRSTREMOV)];		/* what flavor of 215/218 */
	char		c_level;		/* what interrupt level */
};

/*
 * Per-board driver "dynamic" data.
 */

struct	i215state {
	char		s_exists;		/* flag that board exists */
	char		s_state;		/* what just finished (for interrupt) */
	char		s_opunit;		/* current unit being programmed */
	char		s_level;		/* what interrupt level (for i215io) */
	long		s_wua;			/* copy of i215cfg.c_wua */
	char		s_active;		/* per board mutex flags */
	char		s_flags[NUMSPINDLE];	/* flags per spindle; see below */
	char		s_devcode[NUMSPINDLE];	/* device-code for iopb */
	char		s_unit[NUMSPINDLE];	/* "unit" code for iopb */
	char		s_init[NUMSPINDLE];	/* status from init op */
	struct iobuf	*s_bufh;		/* -> buffer header */
	unsigned	s_hcyl;			/* hold cylinder # during restore */
};

/*
 * Per-Unit State Flags.
 */

#define	SF_OPEN		0x01			/* unit is open */
#define	SF_READY	0x02			/* unit is ready; reset by media-change */

/*
 * Values of s_active, used for mutual-exclusion of
 * opens and other IO requests.
 */

#define	IO_IDLE		0x00		/* idle -- anything goes */
#define	IO_OPEN		0x01		/* open waiting */
#define	IO_BUSY		0x02		/* something going on */
#define IO_BACK		0x04		/* seeking backward */
#define IO_HARD_ERR	0x08		/* the last error was a hard error */

/*
 * Macros to make things easier to read/code/maintain/etc...
 */

#define	IO_OP(bp)	((bp->b_flags&B_READ) ? READ_OP : ((bp->b_flags&B_FORMAT) ? FORMAT_OP : WRITE_OP))

/*
 * 215 Wake-Up Block.  Lives at wakeup-address, points at CCB.
 */

struct	i215wub {
	char		w_sysop;	/* Must == 0x01 */
	char		w_rsvd;		/* reserved */
	long 		w_ccb;		/* CCB pointer (8086 style) */
};

/*
 * CCB (Channel-Control-Block).  See 215 manual.
 */

struct	i215ccb {
	char		c_ccw1;		/* 1 ==> Use 215 Firmware */
	char		c_busy1;	/* 0x00 ==> Idle, 0xFF ==> busy */
	long		c_cib;		/* CIB pointer (8086 style) */
	unsigned	c_rsvd0;	/* reserved */
	char		c_ccw2;		/* Must == 0x01 */
	char		c_busy2;	/* Not useful to Host */
	long		c_cpp;		/* -> i215ccb.c_cp (8086 style) */
	unsigned	c_cp;		/* Control Pointer == 0x04 */
};

/*
 * CIB (Channel Invocation Block).  See 215 manual.
 */

struct	i215cib {
	char		c_cmd;		/* reserved */
	char		c_stat;		/* Operation Status (see below) */
	char		c_cmdsem;	/* Not used by 215 */
	char		c_statsem;	/* 0xFF ==> new status avail */
	unsigned	c_csa[2];	/* 215 Firmware; MUST == 0 */
	long		c_iopb;		/* IOPB pointer (8086 style) */
	unsigned	c_rsvd1[2];	/* reserved */
};

/*
 * IOPB (I/O Parameter Block).  See 215 manual.
 */

struct	i215iopb {
	unsigned	i_rsvd[2];	/* reserved */
	unsigned	i_actual;	/* actual transfer count */
	unsigned	i_actfill;	/* fill actual to 32-bits; Unused */
	unsigned	i_device;	/* Device Code (see below) */
	char		i_unit;		/* Unit: <4> == fixed/rem, <1,0> == unit # */
	char		i_funct;	/* Function Code (see below) */
	unsigned	i_modifier;	/* Modifier.  0 ==> normal, interrupt */
	unsigned	i_cylinder;	/* starting cylinder # */
	char		i_head;		/* starting head # */
	char		i_sector;	/* starting sector # */
	long		i_addr;		/* physaddr */
	unsigned	i_xfrcnt;	/* Requested Xfr Count */
	unsigned	i_cntfill;	/* count fill.  Unused */
	unsigned	i_gaddr_ptr[2];	/* general address ptr (not-used) */
};

/*
 * Drive-Data Table. See 215 manual.
 * Note: allignment problem on dr_secsiz. When programming the
 * 	 controller the fields dr_ncyl thru dr_nalt are copied.
 */

struct	i215drtab {
	unsigned	dr_ncyl;	/* # cylinders */
	char		dr_nfhead;	/* # fixed heads */
	char		dr_nrhead;	/* # removable heads */
	char		dr_nsec;	/* # sectors per track */
	unsigned	dr_secsiz;	/* sector size */
	char		dr_nalt;	/* # alternate cylinders */
					/* if floppy, 0==FM, 1==MFM */
	unsigned	dr_spc;		/* actual sectors/cylinder */
	struct i215slice *dr_slice;	/* slice table pointer */
};

/*
 * Local drive-data table. See 215 manual.
 * Note: allignment problem on ldr_secsiz. When programming the
 * 	 controller the fields dr_ncyl thru dr_nalt are copied.
 */

struct	i215ldrtab {
	unsigned	ldr_ncyl;	/* # cylinders */
	char		ldr_nfhead;	/* # fixed heads */
	char		ldr_nrhead;	/* # removable heads */
	char		ldr_nsec;	/* # sectors per track */
	char		ldr_secsiz_l;	/* sector size (low byte) */
	char		ldr_secsiz_h;	/*   "     "   (high byte) */
	char		ldr_nalt;	/* # alternate cylinders */
					/* if floppy, 0==FM, 1==MFM */
};

/*
 * Error Status-Structure, Returned on status inquiry.  See 215 manual.
 * Only 1st 2 fields used.
 * Note: allignment problems on requested cylinder (e_req_cyl_l, e_req_cyl_h),
 *	 and actual cylinder (e_act_cyl_l, e_act_cyl_h), resolved by breaking
 *	 the unsigned integers in to chars.
 */

struct	i215err {
	unsigned	e_hard;		/* Hard Error Status (see below) */
	char		e_soft;		/* soft error status */
	char		e_req_cyl_l;	/* desired cylinder (low byte) */
	char		e_req_cyl_h;	/*    "       "     (high byte) */
	char		e_req_head;	/* desired head and volume */
	char		e_req_sec;	/* desired sector */
	char		e_act_cyl_l;	/* actual cylinder & flags (low byte) */
	char		e_act_cyl_h;	/*   "       "         "   (high byte)*/
	char		e_act_head;	/* actual head & volume */
	char		e_act_sec;	/* actual sector */
	char		e_retries;	/* # retries attempted */
};

/*
 * Format Structure.  1 per "board", usage mutexed via use of "raw" buffer-
 * header.
 * i215ftk is the argument structure to the format ioctl.
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
	daddr_t	f_track;		/* track # */
	int	f_intl;			/* interleave factor */
	int	f_skew;			/* track skew -- ignored by 215 */
	char	f_type;			/* format type-code */
	char	f_pat[4];		/* pattern data */
};

/*
 * error loging stuff
 */
struct	i215eregs {
	struct	i215iopb	e_iopb;
	struct	i215err		e_error;
};

/*
 * 215 Per-Board Device-Data.  One per board (declared in driver).
 */
struct	i215dev {
	struct	i215state	d_state;
	struct	buf		*d_curbuf;
	struct	i215ccb		d_ccb;
	struct	i215cib		d_cib;
	struct	i215iopb	d_iopb;
	struct	i215drtab	*d_drtab[NUMSPINDLE];
	struct	i215eregs	d_eregs;
	struct	i215format	d_format;
	struct	i215ldrtab	d_ldrtab;
};

/*
 * Values of i215state.s_state, internal driver state.
 */

#define	NOTHING			0	/* normal situation, RW */
#define	GET_BAD_STATUS		1	/* retrieveing status; last cmd got error */
#define	RESTORING		2	/* seeking to track 0 for retry */
#define	INITIALIZING		3	/* going thru init-sweep */
#define	READING_LABEL		4	/* reading label for device-characteristics */

/*
 * IOPB fields/flags definitions.
 */

#define	UNIT_REMOVABLE		0x10	/* ==> removable unit */

/*
 * 215 Wake-up command codes.  These get output to the wakeup-address-port.
 */

#define	WAKEUP_CLEAR_INT	0x0000
#define	WAKEUP_START		0x0001
#define	WAKEUP_RESET		0x0002

/*
 * 215 IOPB Command Codes.
 */

#define INIT_OP                 0
#define	STATUS_OP		1
#define	FORMAT_OP		2
#define	READ_ID_OP		3
#define	READ_OP			4
#define	VERIFY_OP		5
#define	WRITE_OP		6
#define	WRITE_BUFFER_OP		7
#define	SEEK_OP			8

/*
 * 215 IOPB Modifier Bits.
 */

#define	MOD_NO_INT		0x0001	/* no interrupt */
#define	MOD_NO_RETRY		0x0002	/* no retry attempts */
#define	MOD_DELETED_DATA	0x0004	/* 218 deleted-data RW */
#define MOD_RESERVED		0x0008	/* reserved don't use */
#define MOD_24_BIT_ADDR		0x0010	/* set 215 to 24 bit address mode */
#define MOD_NO_CLEAR		0x0020	/* 215G no clear the ram on init bit */
#define MOD_LT_STATUS		0x0040	/* 215G tape status for long command */

/*
 * Device Codes (for iopb.i_device).
 */

#define	DEVWINI		0		/* Wini */
#define	DEV8FLPY	1		/* 8" 218 Floppy */
#define	DEV5FLPY	3		/* 5.25" 218 Floppy */
#define DEVWINIG	8		/* 215G, bit 3 set */
#define INVALID		0xF		/* invalid device code */
#define DEVMASK		7		/* to mask of DEVWINIG */


/*
 * Floppy FM/MFM codes for drtab[*].nalt.
 */

#define	FLPY_FM		0		/* FM -- single density */
#define	FLPY_MFM	1		/* MFM -- double density */

/*
 * Operation Status Bits.  Returned by controller in i215cib.c_stat.
 */		

#define	ST_OP_COMPL		0x01	/* operation complete */
#define	ST_SEEK_COMPL		0x02	/* seek complete */
#define	ST_MEDIA_CHANGE		0x04	/* media changed */
#define	ST_FLOPPY		0x08	/* ==> 218 floppy */
#define	ST_UNIT			0x30	/* unit mask */
#define	ST_HARD_ERR		0x40	/* 0 ==> was soft, recovered error */
#define	ST_ERROR		0x80	/* summary error */

/*
 * Error Bits.
 *
 * Errors returned to user in b_error (byte).  Error is either soft-status
 * byte, or high-byte of hard-status byte.  b_error needs to be a word,
 * and can be used as:
 *	Bits	Contents
 *	 6-0	EIO
 *	  7	0 ==> Hard, 1 ==> Soft status
 *	15-8	High-order byte of hard status, or soft status byte.
 */

#define	HARD_WRITE_PROT		0x8000	/* write-protected drive */
#define	HARD_NOT_READY		0x4000	/* went not-ready */
#define	HARD_NO_SECTOR		0x1000	/* couldn't find sector */
#define	SOFT_NO_SECTOR		0x0001	/* set in soft status, "reserved" bit */

/*
 * Misc Format definitions, for i215ftk.f_type.
 */

#define	FORMAT_DATA		0x00	/* format data track */
#define	FORMAT_BAD		0x80	/* format bad track */
#define	FORMAT_ALTERNATE	0x40	/* format alternate track */

/*
 * iSBC 215 ioctl mnemonics.
 */

#define	I215_IOC_FMT		(('W'<<8)|0)
#define I215_FLOPPY		(('W'<<8)|1)
#define I215_CHAR		(('W'<<8)|2)
#define I215_NTRACK		(('W'<<8)|3)
