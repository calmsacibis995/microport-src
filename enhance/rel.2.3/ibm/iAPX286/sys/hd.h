/*#define TVI 0 */
/* uportid = "@(#)hd.h	Microport Rev Id  1.3.8 12/10/86" */
/* Weaver - updated to 386 */
/* names changed to hd-- and 286 - 386 made conditional on MP386	1/7/87 lew	*/

/*
 * hd.h 
 * wd1010, wd1015, wd1014 chip set driver for PCAT
 * Details of the AT Winchester Hardware may be found in the 
 * AT Technical Reference Manual compliments of IBM 
 */
/*
REVISED Larry Weaver 1/22/86 
* M000 lance 4-9-86 removed spl define
*/
#ifdef MP386
#define splbio() spl6()
#define b_secno b_sector
#define WNINTLVL 30		/* Interrupt level */
#else
#define WNINTLVL 46		/* Interrupt level */
#endif
#define I1010RETRY       4              /* retry count */
#define NUMSPINDLE       2              /* # spindles per board */
#define NEXT_REMOVE_UNIT 4              /* increment to next removable unit */
#define ALTTRACKS 60
#define CMOSADR 0x70	/* CMOS RAM address port */
#define CMOSDATA 0x71	/* CMOS RAM data port */
#define bblock hdcache[0]
#define DISKPARMS 0x1a6	/* offset for drive params in master boot blk */
	struct dparams
	{
	short numcyls , 
	numheads, 
	precompcyl, 
	landingzone, 
	checksum ;
	};
/*
 * The minor device number is used as index into the i1010minor array.
 * This array is defined in hd.conf.c. The defines below are to help
 * understand the encoding, but it is still confusing.
 */
#ifdef MP386
#define USRDEV 3
#define TMPDEV -1
#define	ROOTDEV 1
#define SWAPDEV 2
#else
#define USRDEV 2
#define TMPDEV 3
#define	ROOTDEV 0
#define SWAPDEV 1
#endif
/* THESE DEFS HAVE BEEN KLUDGED TO ALLOW THE DRIVE TABLE TO GROW > 16
   The BOARD field has been shortened by two bits; DRTAB extended two */

#define BOARD(dev)      ((i1010minor[minor(dev)]>>14)&0x3)
#define UNIT(dev)       ((i1010minor[minor(dev)]>>10)&0xf)
#define DRTAB(dev)      ((i1010minor[minor(dev)]>>4)&0x3f)
#define SLICE(dev)      ((i1010minor[minor(dev)])&0xf)

#define PHYSADDR( X ) (a20bit ? ADDR86( physaddr( X ) ) : physaddr( X ) ); /* get physical addres for proper board type */

#define i1010MINOR(board,unit,drtab,slice) \
        ((board<<14)|(unit<<10)|(drtab<<4)|slice)

/* makes an 8086 address */
#define ADDR86(X)       ((X&0xffffl)|(((X)<<12)&0xf0000000l))

/*
 * Slice structure.  One per drtab[] entry.
 */

struct  i1010slice {
        daddr_t p_fsec;                 /* first sector */
        daddr_t p_nsec;                 /* number sectors */
};

/*
 * Per controller io address and configuration for wd1010 etc
 */

struct  i1010cfg {
        short             c_io_base;              /* base address of controller */
        char            c_devcod[NUMSPINDLE];   /* total # of units available */
        char            c_level;                /* what interrupt level */
};


/* current cache contents */

struct incache
{
	short in_unit,in_cyl,in_head;
}  ;
/*
 * * Per-board driver "dynamic" data.
 */

struct  i1010state {
        char            s_exists;               /* flag that board exists */
        char            s_state;                /* what just finished (for interrupt) */
        char            s_substate;             /* what just finished (for interrupt) */
        char            s_opunit;               /* current unit being programmed */
        char            s_level;                /* what interrupt level (for i1010io) */
        int             s_io_base;              /* io addr base for this bd */
        char            s_active;               /* per board mutex flags */
        char            s_flags[NUMSPINDLE];    /* flags per spindle; see below */
        char            s_devcode[NUMSPINDLE];  /* device-code for iopb */
        char            s_unit[NUMSPINDLE];     /* "unit" code for iopb */
        char            s_init[NUMSPINDLE];     /* status from init  */
        struct iobuf    *s_bufh;                /* -> buffer header */
        unsigned short       s_timout;		/* timeout indicator */
};

/*
 * Per-Unit State Flags.
 */

#define SF_OPEN         0x01                    /* unit is open */
#define SF_READY        0x02                    /* unit is ready; reset by media-change */
#define SF_VTOC        0x04                    /* unit has VTOC (386 only) */

/*
 * Values of s_active, used for mutual-exclusion of
 * opens and other IO requests.
 */

#define IO_IDLE         0x00            /* idle -- anything goes */
#define IO_OPEN         0x01            /* open waiting */
#define IO_BUSY         0x02            /* something going on */
#define IO_BACK         0x04            /* seeking backward */
#define IO_HARD_ERR     0x08            /* the last error was a hard error */

/*
 * Macros to make things easier to read/code/maintain/etc...
 */

#define IO_OP(bp)       ((bp->b_flags&B_READ) ? READ_OP : ((bp->b_flags&B_FORMAT) ? FORMAT_OP : WRITE_OP))
/* 
	WEAVERS ADDITIONS
*/

#define TASKF (dd->d_state.s_io_base)
#define STATUS TASKF+7
#define HEADHI (TASKF + 0x0206);	/* fixed disk register 03f6 */

/*
 * this is the drive table for the WD1010 devices
 */

struct  i1010drtab {
        unsigned short       dr_ncyl;        /* # cylinders */
        unsigned char   dr_nfhead;      /* # of fixed heads */
        unsigned  short      dr_null0;       /* dos compatable null */
        unsigned  short      dr_precomp;     /* cylinder for precomp */
        unsigned char   dr_null1;       /* dos compatable null */
        unsigned char   dr_control;     /* dos disk control byte */
        unsigned char   dr_null2;       /* dos compatable null */
        unsigned char   dr_null3;       /* dos compatable null */
        unsigned char   dr_null4;       /* dos compatable null */
        unsigned  short    dr_lzone;       /* landing zone for close */
        char            dr_nsec;        /* # sectors per track */
        unsigned char   dr_null5;       /* dos compatable null */
        unsigned short       dr_secsiz;      /* sector size */
        char            dr_nalt;        /* # of alternate cylinders */
        unsigned short       dr_spc;         /* actual sectors/cylinder */
        struct  i1010slice *dr_slice;   /* slice table pointer */
};

/*
 *      char            dr_nrhead;       # removable heads
 */


/*
 * IOPB (I/O Parameter Block).
 */

struct  i1010iopb {
        unsigned        i_rsvd[2];      /* reserved */
        unsigned        i_actual;       /* actual transfer count */
        unsigned        i_usect;      /* user - requested sect */
        unsigned        i_device;       /* Device Code (see below) */
        char            i_unit;    /* Unit: <4> == fixed/rem, <1,0> == unit # */
        char            i_funct;        /* Function Code (see below) */
        unsigned        i_modifier;     /* Modifier.  0 ==> normal, interrupt */
        unsigned        i_cylinder;     /* starting cylinder # */
        char            i_head;         /* starting head # */
        char            i_sector;       /* starting sector # */
        unsigned        i_actcylinder;     /* actual cylinder # */
        char            i_acthead;         /* actual head # */
        long            i_addr;         /* physaddr */
        unsigned        i_xfrcnt;       /* Requested Xfr Count */
        unsigned        i_cntfill;      /* count fill.  */
        long		i_uaddr; /* general address ptr */
};
struct  bad_track_map   {
        unsigned short    bad_cylinder;
        unsigned char   bad_track;
        unsigned short    new_cylinder;
        unsigned char   new_track;
};

struct  bad_tracks      {
        struct  bad_track_map   this_track[86]; /* 6*86 = 516, i.e. >512 */
};

#define INIT_OP                 0
#define STATUS_OP               1
#define FORMAT_OP               2
#define READ_ID_OP              3
#define READ_OP                 4
#define VERIFY_OP               5
#define WRITE_OP                6
#define WRITE_BUFFER_OP         7
#define SEEK_OP                 8

/*
 * Device Codes (for iopb.i_device).
 */

#define DEVWINI         0               /* Wini */
#define INVALID         0x0F            /* bad device */
#define DEVMASK         7               /* mask for device */


/*
 * Floppy FM/MFM codes for drtab[*].nalt.
 *
 * wd1010 error log block
 *
 */

struct  i1010err {
        unsigned        e_hard;         /* Hard Error Status (see below) */
        char            e_soft;         /* soft error status */
        char            e_req_cyl_l;    /* desired cylinder (low byte) */
        char            e_req_cyl_h;    /*    "       "     (high byte) */
        char            e_req_head;     /* desired head and volume */
        char            e_req_sec;      /* desired sector */
        char            e_act_cyl_l;    /* actual cylinder & flags (low byte) */
        char            e_act_cyl_h;    /*   "       "         "   (high byte)*/
        char            e_act_head;     /* actual head & volume */
        char            e_act_sec;      /* actual sector */
        char            e_retries;      /* # retries attempted */
};

/*
 * Format Structure.  1 per "board", usage mutexed via use of "raw" buffer-
 * header.
 * i1010ftk is the argument structure to the format ioctl.
 */

struct  i1010format {
        char    f_trtype;               /* format track-type code */
        char    f_interleave;           /* interleave-factor */
        daddr_t f_trak;
        int     f_hskw;
        int     f_cskw;
};

struct  i1010ftk {
        int	f_cyl;
	int	 f_track;                /* track # */
        int     f_intl;                 /* interleave factor */
        int     f_skew;                 /* track skew by 1010 */
        int     f_cskew;                /* cylinder skew by 1010 */
        char    f_type;               /* format type-code */
};

/*
 * error loging stuff
 */
struct  i1010eregs {
        struct  i1010iopb        e_iopb;
        struct  i1010err         e_error;
};

/*
 * 1010 Per-Board Device-Data.  One per board (declared in driver).
 */
struct  i1010dev {
        struct  i1010state       d_state;
        struct  buf             *d_curbuf;
        struct  i1010iopb       d_iopb;
        struct  i1010drtab      *d_drtab[NUMSPINDLE];
        struct  i1010eregs      d_eregs;
        struct  i1010format     d_format;
};

/*
 * Values of i1010state.s_state, internal driver state.
 */

#define NOTHING                 0       /* normal situation, RW */
#define GET_BAD_STATUS          1       /* retrieveing status; last cmd got error */
#define RESTORING               2       /* seeking to track 0 for retry */
#define INITIALIZING            3       /* going thru init-sweep */
#define READING_LABEL           4       /* reading label for
					 device-characteristics */
#define POLLING			5	/* doing ioctl command */
#define SUBSTATE_SEEK           0x00    /* seek to the track first */
#define SUBSTATE_EXECUTE        0x01    /* perform the operation */

/*
 * IOPB fields/flags definitions.
 */

#define UNIT_REMOVABLE          0x10    /* ==> removable unit */

/*
 * WD1010 Command Codes.
 */

#define WD_RECAL_OP             0x10    /* recalibrate drive */
#define WD_READ_OP              0x20    /* read command */
#define WD_WRITE_OP             0x30    /* write command */

#define WD_VERIFY_OP            0x40    /* read verify with buffer data */
#define WD_FORMAT_TRK_OP        0x50    /* format track command */
#define WD_FORMAT_OP            0x50    /* format track command */
#define WD_SEEK_OP              0x70    /* seek track command */
#define WD_DIAGNOSE_OP          0x90    /* diagnostic command */
#define WD_SET_PARM_OP          0x91    /* initialize drive parameters */
#define WD_NO_RETRY_MOD         0x01    /* turn off retries modifier */
#define WD_ECC_MOD              0x02    /* enable ecc modifier */
#define WD_BUFFER_MODE          0x08    /* enable buffer mode ? */

/*
 * IO Register Definitions for WD1010 on the AT
 */

#define WD0_BASE                0x01F0  /* base io address of wd1010 #1 */
#define WD1_BASE                0x0170  /* base io address of wd1010 #2 */
#define WD0_DATA                WD0_BASE + 0 /* read/write the data buffer */
#define WD0_ERROR               WD0_BASE + 1 /* auxiliary error status reg */
#define WD0_PRECOMP             WD0_BASE + 1 /* precomp write only reg */
#define WD0_SEC_COUNT           WD0_BASE + 2 /* sector count goes here */
#define WD0_SEC_NUMBER          WD0_BASE + 3 /* starting sectorumber */
#define WD0_CYL_LOW             WD0_BASE + 4 /* low byte of cylinder */
#define WD0_CYL_HIGH            WD0_BASE + 5 /* high bits of cyl-1024 max */
#define WD0_SDH                 WD0_BASE + 6 /* size-drive-head */
#define WD0_CMD                 WD0_BASE + 7 /* command-write only */
#define WD0_STAT                WD0_BASE + 7 /* status - read only */
#define WD1_DATA                WD1_BASE + 0 /* read/write the data buffer */
#define WD1_ERROR               WD1_BASE + 1 /* auxiliary error status reg */
#define WD1_PRECOMP             WD1_BASE + 1 /* precomp write only reg */
#define WD1_SEC_COUNT           WD1_BASE + 2 /* sector count goes here */
#define WD1_SEC_NUMBER          WD1_BASE + 3 /* starting sector number */
#define WD1_CYL_LOW             WD1_BASE + 4 /* low byte of cylinder */
#define WD1_CYL_HIGH          WD1_BASE + 5 /* high bits of cyl-1024 max */
#define WD1_SDH                 WD1_BASE + 6 /* size-drive-head */
#define WD1_CMD                 WD1_BASE + 7 /* command-write only */
#define WD1_STAT                WD1_BASE + 7 /* status - read only */

#define WD0_REG_PORT            0x03F6  /* special hardware init reg */
#define WD1_REG_PORT            0x0376  /* hardware init reg for bd # 2 */
#define WD_HDW_RESET            0x04    /* reset the wd hardware */
#define WD_HDW_INIT             0x00    /* initialize the wd hardware */


/*
 * Device Codes (for iopb.i_device).
 */

#define DEVWINI         0               /* Wini */


/*
 * Floppy FM/MFM codes for drtab[*].nalt.
 */

#define FLPY_FM         0               /* FM -- single density */
#define FLPY_MFM        1               /* MFM -- double density */

/*
 *      operational status bit definitions for the WD1010
 */             

#define WD_ST_UNIT              0x04    /* error on last cmd */
#define WD_ST_SEEK_COMPL        0x10    /* error on last cmd */
#define WD_ST_ERROR             0x01    /* error on last cmd */
#define WD_ST_INDEX             0x02    /* index status */
#define WD_ST_CRECTD            0x04    /* ecc correction occured */
#define WD_ST_DRQ               0x08    /* data request to host */
#define WD_ST_SEEK_CMPLT        0x10    /* seek complete status */
#define WD_ST_WRT_FLT           0x20    /* write fault occurred */
#define WD_ST_RDY               0x40    /* wd1010 xfer ready */
#define WD_ST_BSY              0x80    /* wd1010 busy status */

/*
 * Errors returned to user in b_error (byte).  Error is either soft-status
 * byte, or high-byte of hard-status byte.  b_error needs to be a word,
 * and can be used as:
 *      Bits    Contents
 *       6-0    EIO
 *        7     0 ==> Hard, 1 ==> Soft status
 *      15-8    High-order byte of hard status, or soft status byte.
 */
#define WD_ST_HARD_ERR          ERREG_DAM | ERREG_TRK0 | ERREG_ECC
#define HARD_NOT_READY          ERREG_ABORT | ERREG_TRKID
#define ERREG_DAM               0x01    /* data address mark error status */
#define ERREG_TRK0              0x02    /* track 0 fault on seek or recal */
#define ERREG_ABORT             0x04    /* wd1010 cmd or function abort */
#define ERREG_TRKID             0x10    /* track address header mismatch */
#define ERREG_ECC             0x40    /* data field error detected by ecc */
#define ERREG_BAD_BLOCK         0x80    /* bad block was encountered */

/*
 * Misc Format definitions, for i1010ftk.f_type.
 */

#define FORMAT_BAD              0x80    /* format bad track */
#define FORMAT_ALTERNATE        0x40    /* format alternate track */

#define I1010_IOC_FMT           (('W'<<8)|0)
#define I1010_FLOPPY            (('W'<<8)|1)
#define I1010_CHAR              (('W'<<8)|2)
#define I1010_NTRACK            (('W'<<8)|3)
#define I1010_RAWIO             (('W'<<8)|4)
#define I1010_SETDB             (('W'<<8)|5)
#define I1010_REINIT            (('W'<<8)|6)

