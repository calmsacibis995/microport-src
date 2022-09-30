/* @(#)kd_data.h	1.7 */

ushort kb_break_data [NUM_BREAKKEYS] [2] = {
/*    NO-CTRL		CTRL	*/
    { NOKEY, 		NOKEY		},	/*  0 nop		*/
    { SCROLL_KEY, 	BREAK_KEY	},	/*  1 break		*/
    { SCROLL_KEY,	BREAK_KEY	},	/*  2 scroll lock	*/
    { SYSREQ_KEY,	CSIPFX | 'M'	},	/*  3 sysreq		*/
    { SS2CTL | 127,	REBOOT_KEY	},	/*  4 ctrl-alt-del	*/
    { 	   '*',		DEBUG_KEY 	},	/*  5 ctrl-alt-ptrsc	*/
    { DOS_KEY,		CSIPFX | 'M'	},	/*  6 alt-sysreq	*/
    { CONS0_KEY,	SS3PFX | 'P'	},	/*  7 f1		*/
    { CONS0_KEY + 1,	SS3PFX | 'Q'	},	/*  8 f2		*/
    { CONS0_KEY + 2,	SS3PFX | 'R'	},	/*  9 f3		*/
    { CONS0_KEY + 3,	SS3PFX | 'S'	},	/* 10 f4		*/
};

struct keydata kb_std_data [] = {
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },    	/* 00 */
{ 	  0x1b,		0x1b,		0x1b,		0x1b },	/* 01 esc */
{ 	   '1',		 '!',	SS2PFX | '1',	SS2PFX | '!' },	/* 02 1 ! */
{ CTLKEY | '2',	CTLKEY | '@',	SS2PFX | '2',	SS2PFX | '@' },	/* 03 2 @ */
{ 	   '3',		 '#',	SS2PFX | '3',	SS2PFX | '#' },	/* 04 3 # */
{ 	   '4',		 '$',	SS2PFX | '4',	SS2PFX | '$' },	/* 05 4 $ */
{ 	   '5',		 '%',	SS2PFX | '5',	SS2PFX | '%' },	/* 06 5 % */
{ CTLKEY | '6',		 '^',	SS2PFX | '6',	SS2PFX | '^' },	/* 07 6 ^ */
{	   '7',		 '&',	SS2PFX | '7',	SS2PFX | '&' },	/* 08 7 & */
{ 	   '8',		 '*',	SS2PFX | '8',	SS2PFX | '*' },	/* 09 8 * */
{ 	   '9',		 '(',	SS2PFX | '9',	SS2PFX | '(' },	/* 10 9 ( */
{ 	   '0',		 ')',	SS2PFX | '0',	SS2PFX | ')' },	/* 11 0 ) */
{ CTLKEY | '-',		 '_',	SS2PFX | '-',	SS2PFX | '_' },	/* 12 - _ */
{ 	   '=',		 '+',	SS2PFX | '=',	SS2PFX | '+' },	/* 13 = + */
{ 	     8,		   8,		   8,		   8 },	/* 14 bs */
{ 	     9,	CSIPFX | 'Z',		   9,	CSIPFX | 'Z' },	/* 15 tab */
{ ALPKEY | 'q',	ALPKEY | 'Q',	ALPPFX | 'q',	ALPPFX | 'Q' },	/* 16 Q */
{ ALPKEY | 'w',	ALPKEY | 'W',	ALPPFX | 'w',	ALPPFX | 'W' },	/* 17 W */
{ ALPKEY | 'e',	ALPKEY | 'E',	ALPPFX | 'e',	ALPPFX | 'E' },	/* 18 E */
{ ALPKEY | 'r',	ALPKEY | 'R',	ALPPFX | 'r',	ALPPFX | 'R' },	/* 19 R */
{ ALPKEY | 't',	ALPKEY | 'T',	ALPPFX | 't',	ALPPFX | 'T' },	/* 20 T */
{ ALPKEY | 'y',	ALPKEY | 'Y',	ALPPFX | 'y',	ALPPFX | 'Y' },	/* 21 Y */
{ ALPKEY | 'u',	ALPKEY | 'U',	ALPPFX | 'u',	ALPPFX | 'U' },	/* 22 U */
{ ALPKEY | 'i',	ALPKEY | 'I',	ALPPFX | 'i',	ALPPFX | 'I' },	/* 23 I */
{ ALPKEY | 'o',	ALPKEY | 'O',	ALPPFX | 'o',	ALPPFX | 'O' },	/* 24 O */
{ ALPKEY | 'p',	ALPKEY | 'P',	ALPPFX | 'p',	ALPPFX | 'P' },	/* 25 P */
{ CTLKEY | '[',	'{',		SS2PFX | '[',	SS2PFX | '{' },	/* 26 [ { */
{ CTLKEY | ']',	'}',		SS2PFX | ']',	SS2PFX | '}' },	/* 27 ] } */
{ 	  0x0d,	  	 0x0d,		0x0d,		0x0d },	/* 28 return */
{ LCTRL,	LCTRL,		LCTRL,		LCTRL	     },	/* 29 lctrl*/
{ ALPKEY | 'a',	ALPKEY | 'A',	ALPPFX | 'a',	ALPPFX | 'A' },	/* 30 A */
{ ALPKEY | 's',	ALPKEY | 'S',	ALPPFX | 's',	ALPPFX | 'S' },	/* 31 S */
{ ALPKEY | 'd',	ALPKEY | 'D',	ALPPFX | 'd',	ALPPFX | 'D' },	/* 32 D */
{ ALPKEY | 'f',	ALPKEY | 'F',	ALPPFX | 'f',	ALPPFX | 'F' },	/* 33 F */
{ ALPKEY | 'g',	ALPKEY | 'G',	ALPPFX | 'g',	ALPPFX | 'G' },	/* 34 G */
{ ALPKEY | 'h',	ALPKEY | 'H',	ALPPFX | 'h',	ALPPFX | 'H' },	/* 35 H */
{ ALPKEY | 'j',	ALPKEY | 'J',	ALPPFX | 'j',	ALPPFX | 'J' },	/* 36 J */
{ ALPKEY | 'k',	ALPKEY | 'K',	ALPPFX | 'k',	ALPPFX | 'K' },	/* 37 K */
{ ALPKEY | 'l',	ALPKEY | 'L',	ALPPFX | 'l',	ALPPFX | 'L' },	/* 38 L */
{ 	   ';',		 ':',	SS2PFX | ';',	SS2PFX | ':' }, /* 39 ; : */
{ 	   '\'',	 '"',	SS2PFX | '\'',	SS2PFX | '"' },	/* 40 ' " */
{ CTLKEY | '`',		 '~',	SS2PFX | '`',	SS2PFX | '~' },	/* 41 ` ~ */
{ LSHIFT,	LSHIFT,		LSHIFT,		LSHIFT	     }, /* 42 lshift */
{ CTLKEY | '\\',	 '|',	SS2PFX | '\\',	SS2PFX | '|' },	/* 43 \ | */
{ ALPKEY | 'z',	ALPKEY | 'Z',	ALPPFX | 'z',	ALPPFX | 'Z' },	/* 44 Z */
{ ALPKEY | 'x',	ALPKEY | 'X',	ALPPFX | 'x',	ALPPFX | 'X' },	/* 45 X */
{ ALPKEY | 'c',	ALPKEY | 'C',	ALPPFX | 'c',	ALPPFX | 'C' },	/* 46 C */
{ ALPKEY | 'v',	ALPKEY | 'V',	ALPPFX | 'v',	ALPPFX | 'V' },	/* 47 V */
{ ALPKEY | 'b',	ALPKEY | 'B',	ALPPFX | 'b',	ALPPFX | 'B' },	/* 48 B */
{ ALPKEY | 'n',	ALPKEY | 'N',	ALPPFX | 'n',	ALPPFX | 'N' },	/* 49 N */
{ ALPKEY | 'm',	ALPKEY | 'M',	ALPPFX | 'm',	ALPPFX | 'M' },	/* 50 M */
{ 	   ',',		 '<', 	SS2PFX | ',',	SS2PFX | '<' },	/* 51 ,	< */
{ 	   '.',		 '>', 	SS2PFX | '.',	SS2PFX | '>' },	/* 52 . > */
{ CTLKEY | '/',		 '?',	SS2PFX | '/',	SS2PFX | '?' },	/* 53 / ? */
{ RSHIFT,	RSHIFT,		RSHIFT,		RSHIFT	     },	/* 54 rshift */
{ 	   '*',	CSIPFX | 'i',	DEBUG_KEY,	SS2PFX | '*' },	/* 55 prtsc * */
{ LALT,		LALT,		LALT,		LALT	     },	/* 56 left alt*/
{ CTLKEY | ' ',	CTLKEY | ' ',	CTLKEY | ' ',	CTLKEY | ' ' },	/* 57 space */
{ CAPSLOCK,	CAPSLOCK,	CAPSLOCK,	CAPSLOCK     },	/* 58 capslock*/
#ifdef	ATTCOMP
{ SS3PFX | 'P',	SS3PFX | 'p',	SS3PFX | 'P',	SS3PFX | 'p' },	/* 59 F1 */
{ SS3PFX | 'Q',	SS3PFX | 'q',	SS3PFX | 'Q',	SS3PFX | 'q' },	/* 60 F2 */
{ SS3PFX | 'R',	SS3PFX | 'r',	SS3PFX | 'R',	SS3PFX | 'r' },	/* 61 F3 */
{ SS3PFX | 'S',	SS3PFX | 's',	SS3PFX | 'S',	SS3PFX | 's' },	/* 62 F4 */
#else
{ SS3CTL | 'P',	SS3PFX | 'p',	CONS0_KEY,	SS3PFX | 'p' },	/* 59 F1 */
{ SS3CTL | 'Q',	SS3PFX | 'q',	CONS0_KEY + 1,	SS3PFX | 'q' },	/* 60 F2 */
{ SS3CTL | 'R',	SS3PFX | 'r',	CONS0_KEY + 2,	SS3PFX | 'r' },	/* 61 F3 */
{ SS3CTL | 'S',	SS3PFX | 's',	CONS0_KEY + 3,	SS3PFX | 's' },	/* 62 F4 */
#endif
{ SS3PFX | 'T',	SS3PFX | 't',	SS3PFX | 'T',	SS3PFX | 't' },	/* 63 F5 */
{ SS3PFX | 'U',	SS3PFX | 'u',	SS3PFX | 'U',	SS3PFX | 'u' },	/* 64 F6 */
{ SS3PFX | 'V',	SS3PFX | 'v',	SS3PFX | 'V',	SS3PFX | 'v' },	/* 65 F7 */
{ SS3PFX | 'W',	SS3PFX | 'w',	SS3PFX | 'W',	SS3PFX | 'w' },	/* 66 F8 */
{ SS3CTL | 'X',	SS3PFX | 'x',	SS3PFX | 'X',	SS3PFX | 'x' },	/* 67 F9 */
{ SS3PFX | 'Y',	SS3PFX | 'y',	SS3PFX | 'Y',	SS3PFX | 'y' },	/* 68 F10 */
{ NUMLOCK,	NUMLOCK,	NUMLOCK,	NUMLOCK	     },	/* 69 numlock */
#ifdef	ATTCOMP
{ CSIPFX | 'M',	BREAKKEY,	CSIPFX | 'M',	BREAKKEY     },	/* 70 scroll */
#else
{ SCROLL_KEY,	BREAK_KEY,	CSIPFX | 'M',	BREAK_KEY    },	/* 70 scroll */
#endif
{ KEYPAD | 'H',	NUMLCK | '7',	KEYPAD | 'H',	KEYPD2 | '7' },	/* 71 home */
{ KEYPAD | 'A',	NUMLCK | '8',	KEYPAD | 'A',	KEYPD2 | '8' },	/* 72 up */
{ KEYPAD | 'V',	NUMLCK | '9',	KEYPAD | 'V',	KEYPD2 | '9' },	/* 73 pg up */
#ifdef	ATTCOMP
{ CSIPFX | 'S',	         '-',	CSIPFX | 'S',	SS2PFX | '-' },	/* 74 minus */
#else
{          '-',	         '-',	CSIPFX | 'S',	SS2PFX | '-' },	/* 74 minus */
#endif
{ KEYPAD | 'D',	NUMLCK | '4',	KEYPAD | 'D',	KEYPD2 | '4' },	/* 75 left */
{ KEYPAD | 'G',	NUMLCK | '5',	KEYPAD | 'G',	KEYPD2 | '5' },	/* 76 5 */
{ KEYPAD | 'C',	NUMLCK | '6',	KEYPAD | 'C',	KEYPD2 | 'C' },	/* 77 right */
#ifdef	ATTCOMP
{ CSIPFX | 'T',	         '+',	CSIPFX | 'T',	SS2PFX | '+' },	/* 78 plus */
#else
{          '+',	         '+',	CSIPFX | 'T',	SS2PFX | '+' },	/* 78 plus */
#endif
{ KEYPAD | 'Y',	NUMLCK | '1',	KEYPAD | 'Y',	KEYPD2 | '1' },	/* 79 end */
{ KEYPAD | 'B',	NUMLCK | '2',	KEYPAD | 'B',	KEYPD2 | '2' },	/* 80 down */
{ KEYPAD | 'U',	NUMLCK | '3',	KEYPAD | 'U',	KEYPD2 | '3' },	/* 81 pg dn */
{ KEYPAD | '@',	NUMLCK | '0',	KEYPAD | '@',	KEYPD2 | '0' },	/* 82 insert */
{ NUMLCK | 127,	NUMLCK | '.',	REBOOT_KEY,	KEYPD2 | '.' },	/* 83 del */
#ifdef	ATTCOMP
{ CSIPFX | 'L',	CSIPFX | 'M',	CSIPFX | 'L',	CSIPFX | 'M' },	/* 84 sysreq */
#else
{ SYSREQ_KEY,	CSIPFX | 'L',	DOS_KEY,	CSIPFX | 'M' }, /* 84 sysreq */
#endif
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 85 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 86 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 87 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 88 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 89 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 90 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 91 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 92 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 93 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 94 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 95 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 96 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 97 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 98 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 99 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 100 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 101 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 102 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 103 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 104 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 105 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 106 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 107 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 108 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 109 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 110 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 111 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 112 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 113 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 114 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 115 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 116 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 117 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 118 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 119 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 120 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 121 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 122 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 123 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 124 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 125 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 126 */
{ NOKEY,	NOKEY,		NOKEY,		NOKEY },	/* 127 */
};

unsigned char kb_std_control [] = {
	0,	/* 00 */
	0x1b,	/* 01 esc */
	0,	/* 02 1 ! */
	0x00,	/* 03 2 @ */
	0,	/* 04 3 # */
	0,	/* 05 4 $ */
	0,	/* 06 5 % */
	0x1e,	/* 07 6 ^ */
	0,	/* 08 7 & */
	0,	/* 09 8 * */
	0,	/* 10 9 ( */
	0,	/* 11 0 ) */
	0x1f,	/* 12 - _ */
	0,	/* 13 = + */
	0,	/* 14 bs */
	0,	/* 15 tab */
	0x11,	/* 16 Q */
	0x17,	/* 17 W */
	0x05,	/* 18 E */
	0x12,	/* 19 R */
	0x14,	/* 20 T */
	0x19,	/* 21 Y */
	0x15,	/* 22 U */
	0x09,	/* 23 I */
	0x0f,	/* 24 O */
	0x10,	/* 25 P */
	0x1b,	/* 26 [ { */
	0x1d,	/* 27 ] } */
	0,	/* 28 return */
	0,	/* 29 lctrl*/
	0x01,	/* 30 A */
	0x13,	/* 31 S */
	0x04,	/* 32 D */
	0x06,	/* 33 F */
	0x07,	/* 34 G */
	0x08,	/* 35 H */
	0x0a,	/* 36 J */
	0x0b,	/* 37 K */
	0x0c,	/* 38 L */
	0,	/* 39 ; : */
	0,	/* 40 ' " */
	0x1e,	/* 41 ` ~ */
	0,	/* 42 lshift */
	0x1c,	/* 43 \ | */
	0x1a,	/* 44 Z */
	0x18,	/* 45 X */
	0x03,	/* 46 C */
	0x16,	/* 47 V */
	0x02,	/* 48 B */
	0x0e,	/* 49 N */
	0x0d,	/* 50 M */
	0,	/* 51 , < */
	0,	/* 52 . > */
	0x1f,	/* 53 / ? */
	0,	/* 54 rshift */
	'*',	/* 55 prtsc * */
	0,	/* 56 left alt*/
	0,	/* 57 space */
	0,	/* 58 capslock*/
#ifdef	ATTCOMP
	0x50,	/* 59 F1 */
	0x51,	/* 60 F2 */
	0x52,	/* 61 F3 */
	0x53,	/* 62 F4 */
	0x54,	/* 63 F5 */
	0x55,	/* 64 F6 */
	0x56,	/* 65 F7 */
	0x57,	/* 66 F8 */
	0x58,	/* 67 F9 */
	0x59,	/* 68 F10 */
#else
	0x10,	/* 59 F1 */
	0x11,	/* 60 F2 */
	0x12,	/* 61 F3 */
	0x13,	/* 62 F4 */
	0x14,	/* 63 F5 */
	0x15,	/* 64 F6 */
	0x16,	/* 65 F7 */
	0x17,	/* 66 F8 */
	0x18,	/* 67 F9 */
	0x19,	/* 68 F10 */
#endif
	0,	/* 69 numlock */
	0,	/* 70 scroll */
	0,	/* 71 home */
	0,	/* 72 up */
	0,	/* 73 pg up */
	0,	/* 74 minus */
	0,	/* 75 left */
	0,	/* 76 5 */
	0,	/* 77 right */
	0,	/* 78 plus */
	0,	/* 79 end */
	0,	/* 80 down */
	0,	/* 81 pg dn */
	0,	/* 82 insert */
	'.',	/* 83 del */
	0x0c,	/* 84 sysreq */
	0,	/* 85 */
	0,	/* 86 */
	0,	/* 87 */
	0,	/* 88 */
	0,	/* 89 */
	0,	/* 90 */
	0,	/* 91 */
	0,	/* 92 */
	0,	/* 93 */
	0,	/* 94 */
	0,	/* 95 */
	0,	/* 96 */
	0,	/* 97 */
	0,	/* 98 */
	0,	/* 99 */
	0,	/* 100 */
	0,	/* 101 */
	0,	/* 102 */
	0,	/* 103 */
	0,	/* 104 */
	0,	/* 105 */
	0,	/* 106 */
	0,	/* 107 */
	0,	/* 108 */
	0,	/* 109 */
	0,	/* 110 */
	0,	/* 111 */
	0,	/* 112 */
	0,	/* 113 */
	0,	/* 114 */
	0,	/* 115 */
	0,	/* 116 */
	0,	/* 117 */
	0,	/* 118 */
	0,	/* 119 */
	0,	/* 120 */
	0,	/* 121 */
	0,	/* 122 */
	0,	/* 123 */
	0,	/* 124 */
	0,	/* 125 */
	0,	/* 126 */
	0,	/* 127 */
};
