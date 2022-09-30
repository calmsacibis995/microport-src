/* @(#)kd_info.h	1.2 Microport 9/23/87 */

#define	NUMCONSOLES	16	/* number of allowable consoles */

/* signal data structure */
union sigkeydata {
    struct {
	short data;		/* user specified data	*/
	short active;		/* current active page	*/
    } key;
    struct {
	short parent;		/* father of this screen */
	short child;		/* who we inherit	*/
    } link;
    unsigned long iocparm;	/* convienent for caller */
    unsigned char *iocptr;	/* convienent for callee */
};

/* signal key structure */
struct setsigkey {
    int   k_signal;	/* which signal to send		*/
    short k_data;	/* data returned to process	*/
    short k_pid;	/* process id to send it to	*/
    char  k_code;	/* character code		*/
    char  k_shift;	/* shift type			*/
};

/* reserved scan codes */
#define	F1_SCANCODE	59

/* augument table selectors */
#define	K_CTRLTAB 4		/* select control table		*/

/* augument character table key types */
#define	CS2PFX	0x0600		/* prefix key with <ESC> [ !	*/
#define	DOSKEY	0x0e00		/* prefix key with '\0'		*/

/* additional IOCTL's */
#define	KDIOC(x)	('K'<<8 | 0x40 | (x))
#define	KDDELECONS	KDIOC (1)	/* delete console	*/
#define	KDADDCONS	KDIOC (2)	/* add a console	*/
#define	KDCHANGECONS	KDIOC (3)	/* change console	*/
#define	KDSETLASTACTIVE	KDIOC (4)	/* set last active	*/
#define	KDCLOSEGRAPHICS	KDIOC (5)	/* shut down graphics	*/
#define	KDGETBUFFSIZE	KDIOC (6)	/* get graphics buffer size */
#define	KDSETPARENT	KDIOC (7)	/* set parent screen	*/
#define	KDSETCHILD	KDIOC (8)	/* set child screen	*/

#define	KDSETSIGKEY	KDIOC (9)	/* set signal key	*/
#define	KDGETSIGKEY	KDIOC (10)	/* get signal key data	*/
#define	KDCLRSIGKEY	KDIOC (11)	/* clear signal key	*/

/* augument LED definitions */
#define	LED_SCROLL	0x01	/* SCROLL LOCK led		*/

/* augument keyboard controller commands */
#define	KB_DISABKB	0xAD	/* disable keyboard		*/

/* augment keyboard shift state variable */
#define	SCROLL_LOCK	0x100	/* scroll state is set		*/

/* key combinations and abbreviations */
#define	KEYPAD	NUMLCK | CSIPFX		/* numeric key & csi prefix */
#define	KEYPD2	NUMLCK | SS2PFX		/* numeric key & ss2 prefix */
#define	ALPKEY	CTLKEY | CAPLCK		/* control key & shiftable */
#define	ALPPFX	CTLKEY | CAPLCK | SS2PFX /* control & shiftable & ss2 prefix */
#define	SS2CTL	SS2PFX | CTLKEY		/* ss2 prefix & control key */
#define	SS3CTL	SS3PFX | CTLKEY		/* ss3 prefix & control key */

/* shift directives */
#define	LCTRL	 SHIFTKEY | LEFT_CTRL
#define	RCTRL	 SHIFTKEY | RIGHT_CTRL
#define	LSHIFT	 SHIFTKEY | LEFT_SHIFT
#define	RSHIFT	 SHIFTKEY | RIGHT_SHIFT
#define	LALT	 SHIFTKEY | LEFT_ALT
#define	RALT	 SHIFTKEY | RIGHT_ALT
#define	CAPSLOCK SHIFTKEY | CAPS_LOCK
#define	NUMLOCK	 SHIFTKEY | NUM_LOCK

/* special break keys */
#define	NOP_REQUEST	0
#define	BREAK_REQUEST 	1
#define	SCROLL_REQUEST	2
#define	SYS_REQUEST	3
#define	SYS_REBOOT	4
#define	DEBUG_REQUEST	5
#define	DOS_SWITCH	6
#define	CONS_0		7
#define	NUM_BREAKKEYS	(CONS_0+NUMCONSOLES)

#define	BREAK_KEY	BREAKKEY | BREAK_REQUEST
#define	SCROLL_KEY	BREAKKEY | SCROLL_REQUEST
#define	SYSREQ_KEY	BREAKKEY | SYS_REQUEST
#define	REBOOT_KEY	BREAKKEY | NUMLCK | SYS_REBOOT
#define	DEBUG_KEY	BREAKKEY | DEBUG_REQUEST
#define	DOS_KEY		BREAKKEY | DOS_SWITCH
#define	CONS0_KEY	BREAKKEY | CONS_0

/* keyboard controller constants */
#define KB_VEC		0x21	/* physical keyboard vector */
#define KB_DISABLE	0xAD	/* disable keyboard command */

#define KB_RESEND	0xFE	/* keyboard resend response */

/* Kb_Status bits */
#define KB_ACK_FLG 0x10		/* keyboard acknowledge flag for mode ind */
#define KB_RES_FLG 0x20		/* keyboard resend flag for mode indicator */
#define KB_LED_FLG 0x40		/* LED update in progress flag  for mode i */
#define KB_ERR_FLG 0x80		/* transmit error flag for mode indicator */

#define T10mS 0x1000		/* timedelay for 10 milliseconds */
#define RETRYCNT 3		/* number of retries for commands to keybrd */

/* ... */
