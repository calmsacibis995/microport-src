/* uportid = "@(#)cmos.h	Microport Rev Id  1.3.7 8/11/86" */
/* cmos.h:
 * 
 * The layout of cmos for the IBM AT. See pps 1-45 to 1-55 of
 * the IBM Technical Reference Manual.
 */

#define	CMOSSIZE	0x40			/* # of bytes in cmos	*/
#define	CMOSDEV		"/dev/cmos"		/* where cmos lives	*/

struct cmos {					/* the layout of cmos	*/
						/* realtime clock info	*/
		unsigned char sec;		/* 0: seconds		*/
		unsigned char secalarm;		/* 1: second alarm	*/
		unsigned char min;		/* 2: minutes		*/
		unsigned char minalarm;		/* 3: minute alarm	*/
		unsigned char hours;		/* 4: hours		*/
		unsigned char houralarm;	/* 5: hour alarm	*/
		unsigned char weekday;		/* 6: day of week	*/
		unsigned char monthday;		/* 7: day of month	*/
		unsigned char month;		/* 8: month		*/
		unsigned char year;		/* 9: year		*/

						/* status registers	*/
		unsigned char statusa;		/* A: status reg.A	*/
		unsigned char statusb;		/* B: status reg.B	*/
		unsigned char statusc;		/* C: status reg.C	*/
		unsigned char statusd;		/* D: status reg.D	*/
		unsigned char diagsts;		/* E: diagnostic status byte*/
		unsigned char shutdown;		/* F: shutdown status byte */

						/* device characteristics */
		unsigned char diskette;		/* 10: floppy drives:A&B*/
		unsigned char res1;		/* 11: unused		*/
		unsigned char disk;		/* 12: fixed disk: C & D*/
		unsigned char res2;		/* 13: unused		*/
		unsigned char equip;		/* 14: equipment byte	*/
		unsigned char lowbase;		/* 15: low base mem byte*/
		unsigned char hibase;		/* 16: hi base mem byte	*/
		unsigned char lowexp1;		/* 17: low exp. mem byte*/
		unsigned char hiexp1;		/* 18: hi exp. mem byte */
		unsigned char diskC;		/* 19: disk type ext.: C*/
		unsigned char diskD;		/* 1A: disk type ext.: D*/
		unsigned char res3[18];		/* 1B: unused		*/
		unsigned int chksum;		/* 2E-2F: cmos checksum	*/
		unsigned char lowexp2;		/* 30: low exp. mem byte*/
		unsigned char hiexp2;		/* 31: hi exp. mem byte */
		unsigned char century;		/* 32: date century byte*/
		unsigned char infoflags;	/* 33: information flags*/
		unsigned char res4[11];		/* 34-3F: unused	*/
} cmos;
