#ifdef	MP386
#ident "@(#)kd.h	1.1"
#else
typedef	unsigned char	unchar;
#endif
/*
 * definitions for PC AT keyboard/display driver
 */

/*
 * keyboard controller I/O port addresses
 */
#define KB_OUT	0x60		/* output buffer R/O */
#define KB_IDAT 0x60		/* input buffer data write W/O */
#define KB_STAT 0x64		/* keyboard controller status R/O */
#define KB_ICMD 0x64		/* input buffer command write W/O */

/*
 * keyboard controller commands and flags
 */
#define KB_INBF		0x02	/* input buffer full flag */
#define KB_OUTBF	0x01	/* output buffer full flag */
#define KB_GATE20	0x02	/* set this bit to allow addresses > 1Mb */
#define KB_ROP		0xD0	/* read output port command */
#define KB_WOP		0xD1	/* write output port command */
#define	KB_RCB		0x20	/* read command byte command */
#define	KB_WCB		0x60	/* write command byte command */
#define	KB_ENAB		0xae	/* enable keyboard interface */
#define	KB_INHIB	0x10	/* keyboard inhibited */
#define	KB_DISAB	0x10	/* disable keyboard */
#define	KB_EOBFI	0x01	/* enable interrupt on output buffer full */
#define	KB_INOV		0x08	/* override keyboard inhibit function */
#define KB_ACK		0xFA	/* Acknowledgement byte from keyboard */

/*
 * The keyboard translation tables are laid out as follows:
 * There are four tables, one for unaugmented keypresses,
 * one for shifted keypresses, one for alted keypresses, and
 * one for shift-alted keypresses.  Each table has 128 16 bit 
 * entries, containing flags in the top 8 bits and the character
 * to send in the lower 8 bits.
 *
 *	 1 1 1   1
 *	 5 4 3   1  8 7      0
 *	+-+-+-+-+----+--------+
 *	|n|c|t| |type| char   |
 *      +-+-+-+-+----+--------+
 *
 *	n - key affected by num lock flag
 *	c - key affected by caps lock flag
 *	t - key affected by control key flag
 *	type - type of key, prefix specifier
 *	char - character to send
 *
 */
/*
 * keyboard translation modes
 */
#define	K_RAW		0x00	/* keyboard just sends raw up/down scan codes */
#define	K_XLATE		0x01	/* keyboard translates scan codes to ascii */
/*
 * table selectors
 */
#define	K_NORMTAB	0x00	/* select unaugmented keyboard table */
#define	K_SHIFTTAB	0x01	/* select shifted keyboard table */
#define	K_ALTTAB	0x02	/* select alted keyboard table */
#define	K_ALTSHIFTTAB	0x03	/* select alt-shifted keyboard table */
/*
 * character table flag bits.
 */
#define	NUMLCK		0x8000	/* flag key as affected by num lock key */
#define	CAPLCK		0x4000	/* flag key as affected by caps lock key */
#define	CTLKEY		0x2000	/* flag key as afected by control keys */
/*
 * character table key types
 */
#define	NORMKEY		0x0000	/* key is a normal key, send with no prefix */
#define	SHIFTKEY	0x0100	/* key is a shift key */
#define	BREAKKEY	0x0200	/* key is a break key */
#define	SS2PFX		0x0300	/* prefix key with <ESC> N */
#define	SS3PFX		0x0400	/* prefix key with <ESC> O */
#define	CSIPFX		0x0500	/* prefix key with <ESC> [ */
#define	NOKEY		0x0f00	/* flag a key as nonexistant */
#define	TYPEMASK	0x0f00	/* select key type bits */

/*
 * kb_state bit definitions
 */
#define LEFT_SHIFT	0x01	/* left shift key depressed */
#define	LEFT_ALT	0x02	/* left alt key depressed */
#define	LEFT_CTRL	0x04	/* left control key depressed */
#define	RIGHT_SHIFT	0x08	/* right shift key depressed */
#define	RIGHT_ALT	0x10	/* right alt key depressed */
#define	RIGHT_CTRL	0x20	/* right control key depressed */
#define	CAPS_LOCK	0x40	/* caps lock key down */
#define	NUM_LOCK	0x80	/* num lock key down */
#define	ALTSET		(LEFT_ALT|RIGHT_ALT)
#define	SHIFTSET	(LEFT_SHIFT|RIGHT_SHIFT)
#define	CTRLSET		(LEFT_CTRL|RIGHT_CTRL)
#define	NONTOGGLES	(ALTSET|SHIFTSET|CTRLSET)

/* keyboard scan codes */
#define SYS_REQ	0x54		/* system request key */
#define DEL_KEY 0x53		/* delete key */

#define SIZETTY (sizeof(struct tty))

/*
 * character flags, should not conflict with FRERROR and friends
 * in tty.h
 */
#define GEN_ESCLSB	0x0800		/* generate <ESC> [ prefix to char */
#define GEN_ESCN	0x0400		/* generate <ESC> N prefix to char */
#define GEN_ESCO	0x0200		/* generate <ESC> O prefix to char */
#define NO_CHAR		0x8000		/* did not generate a char */

#define KDRGSIZ	16		/* size of kdrgbuf, the circular buffer */
#define MAXTAB	10		/* maximum number of tab stops */

/* Definitions to support the LED on the PC-AT */
#define LED_WARN	0xED	/* Tell kbd that following byte is led status */
#define LED_SEND	0x01	/* Send led status bit */
#define LED_CAP		0X04	/* Flag bit for cap lock */
#define LED_NUM		0x02	/* Flag bit for num lock */

/* Screen definitions */
#define MAXPARAMS	5	/* maximum number of ANSI parameters */
#define LINELEN		80
#define HEIGHT		25

/*
 * type of adapter installed, matches bits in CMOS ram
 */
#define	MCAP_UNK	0xff	/* adapter not determined yet */
#define MCAP_MONO	0x03	/* mono adaptor installed */
#define MCAP_COLOR	0x02	/* color adaptor installed in 80 column mode */
#define MCAP_COLOR40	0x01	/* color adaptor installed in 40 column mode */
#define MCAP_EGA	0x00	/* EGA adaptor installed */

/* offsets from 6845 base address for various registers */
#define DATA_REG	0x1
#define MODE_REG	0x4
#define COLOR_REG	0x5	/* color adapter only */
#define STATUS_REG	0x6

/* definitions for bits in ad_colmode */
#define MODE_TYPE	0x07   	/* mask for alpha/graphic modes */
#define MODE_GRAPH	0x04  	/* graphics modes */
#define MODE_40		0x00	/* 40x25 alphanumeric */
#define MODE_80		0x01	/* 80x25 alphanumeric */
#define MODE_GRLRES	0x04	/* 160x100 graphics */
#define MODE_GRMRES	0x05	/* 320x200 graphics */
#define MODE_GRHRES	0x06	/* 640x200 graphics */
#define MODE_BLINK	0x00	/* 8 background colors, blink */
#define MODE_BG16	0x08	/* 16 background colors, no blink */
#define MODE_CO		0x00	/* color adaptor, enable color */
#define MODE_BW		0x10	/* color adaptor, disable color */

/* definitions for bits in the color adapter mode register */
#define M_ALPHA40	0x00	/* 40 by 25 alphanumeric */
#define M_ALPHA80	0x01	/* 80 by 25 alphanumeric */
#define M_GRAPH		0x02	/* 320x200 or 640x200 graphics */
#define M_BW		0x04	/* black & white */
#define M_ENABLE	0x08	/* video enable */
#define M_HIGHRES	0x10	/* 640x200 B&W graphics */
#define M_BLINK		0x20	/* enable blink attribute */

/* definitions for bits in the color adapter status register */
#define S_UPDATEOK	0x01	/* safe to update regen buffer */
#define S_VSYNC		0x08	/* raster is in vertical retrace mode */

/* definitions for loading data into 6845 registers */
#define R_NUMREGS	16	/* number of regs to initialize */
#define R_STARTADRH	12	/* start address, high word */
#define R_STARTADRL	13	/* start address, low word */
#define R_CURADRH	14	/* cursor address, high word */
#define R_CURADRL	15	/* cursor address, low word */

/* definitions for bits in the attribute byte */
#define BLINK		0x80
#define BRIGHT		0x08
#define REVERSE		0x70
#define NORM		0x07
#define UNDERLINE	0x01	/* underline on mono, blue on color */

#define CLEAR		NORM<<8|0x20
#define BCLEAR		NORM|BRIGHT<<8|0x20
#define ALLATTR		BLINK|BRIGHT|REVERSE|NORM|UNDERLINE
#define NOTBGRND	ALLATTR&(~REVERSE)
#define NOTFGRND	ALLATTR&(~NORM)

/* definitions for ringing the bell */
#define NORMBELL	1331	/* initial value loaded into timer */
#define BELLLEN		10	/* ring for 10/60 sec. between checks */
#define BELLCNT		2	/* check bell twice before turning off */
#define TONE_ON		3	/* 8254 gate 2 and speaker and-gate enabled */
#define TIMER		0x40	/* 8254.2 timer address */
#define TIMERCR		TIMER+3	/* timer control register address */
#define TIMER2		TIMER+2	/* timer tone generation port */
#define T_CTLWORD	0xB6	/* value for timer control word */
#define TONE_CTL	0x61	/* address for enabling timer port 2 */

/* defines for keyboard and display ioctl's */
#define KIOC		('K'<<8)
#define KDGMODE		(KIOC|1)	/* gets adapter mode */
#define KDSMODE		(KIOC|2)	/* sets adapter mode */
#define KDSCRCTRL	(KIOC|3)	/* turn display on/off */
#define KDSCRATTCH	(KIOC|4)	/* attach display memory to user proc */
#define	KDGKBENT	(KIOC|5)	/* get keyboard table entry */
#define	KDSKBENT	(KIOC|6)	/* set keyboard table entry */
#define	KDGKBMODE	(KIOC|7)	/* get keyboard translation mode */
#define	KDSKBMODE	(KIOC|8)	/* set keyboard translation mode */


struct adtstruct {
	struct tty	*ad_ttyp;	/* pointer to tty struct */
	ushort	*ad_scraddr;	/* address of adaptor memory */
	ushort	ad_scrmask;	/* size-1 in shorts of adaptor memory */
	ushort	ad_adr6845;	/* address of corresponding M6845 */
	ushort	ad_width;	/* number of characters horizontally */
	ushort	ad_height;	/* number of characters vertically */
	ushort	ad_scrsize;	/* number of characters on screen */
	short	ad_line;	/* current line number */
	short	ad_column;	/* current column number */
	ushort	ad_cursor;	/* current address of cursor, 0-based */
	ushort	ad_curbase;	/* ulhc of screen relative to scraddr */
	unchar	ad_capability;
	unchar	ad_colmode;	/* color adapter mode */
	unchar	ad_colsel;	/* color select register byte */
	unchar	ad_modesel;	/* mode register byte */
	unchar	ad_state;	/* state in output esc seq processing */
	unchar	ad_undstate;	/* underline processing state */
	unchar	ad_attribute;	/* current attribute for characters */
	unchar	ad_gotparam;	/* does output esc seq have a param */
	unchar	ad_undattr;	/* attribute to use when underlining */
	ushort	ad_curparam;	/* current param # of output esc seq */
	ushort	ad_paramval;	/* value of current param */
	short	ad_params[MAXPARAMS];	/* parameters of output esc seq */
	short	ad_tabs[MAXTAB];	/* list of tab stops */
};


struct attrmask {
	unchar attr;		/* new attribute to turn on */
	unchar mask;		/* old attributes to leave on */
};


struct m6845init {
	unchar  mi_hortot;   /* Reg  0: Horizontal Total     (chars) */
	unchar  mi_hordsp;   /* Reg  1: Horizontal Displayed (chars) */
	unchar  mi_hsnpos;   /* Reg  2: Hsync Position       (chars) */
	unchar  mi_hsnwid;   /* Reg  3: Hsync Width          (chars) */
	unchar  mi_vertot;   /* Reg  4: Vertical Total   (char rows) */
	unchar  mi_veradj;   /* Reg  5: Vtotal Adjust   (scan lines) */
	unchar  mi_vsndsp;   /* Reg  6: Vertical Display (char rows) */
	unchar  mi_vsnpos;   /* Reg  7: Vsync Position   (char rows) */
	unchar  mi_intlac;   /* Reg  8: Interlace Mode               */
	unchar  mi_maxscn;   /* Reg  9: Max Scan Line   (scan lines) */
	unchar  mi_curbeg;   /* Reg 10: Cursor Start    (scan lines) */
	unchar  mi_curend;   /* Reg 11: Cursor End      (scan lines) */
	unchar  mi_stadh;    /* Reg 12: Start Address (H)            */
	unchar  mi_stadl;    /* Reg 13: Start Address (L)            */
	unchar  mi_cursh;    /* Reg 14: Cursor (H)                   */
	unchar  mi_cursl;    /* Reg 15: Cursor (L)                   */
};

struct adtmode {
	unchar	am_capability;
	unchar	am_colmode;
	unchar	am_colsel;
};

struct kbentry {
	unchar	kb_table;	/* which table to use */
	unchar	kb_index;	/* which entry in table */
	ushort	kb_value;	/* value to get/set in table */
};

