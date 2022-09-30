/*
 * Header file for the speaker I/O driver - Marc de Groot
 */

/*
 * Port addresses
 */
#define SP_TIMER 0x40			/* speaker timer port base addr */
#define SP_PORTB 0x61			/* speaker gate port addr */

/*
 * ioctl function codes
 */
#define SP_SET_DIVISOR		(('S' << 8) | 0)
#define SP_TURN_ON		(('S' << 8) | 1)
#define SP_SET_TICKS		(('S' << 8) | 2)
#define SP_SET_FREQUENCY	(('S' << 8) | 3)

/*
 * constants for the state machine
 */
#define SP_OCTAVE	20
#define SP_NOTE_LEN	21
#define SP_NOTENM1	22
#define SP_NOTENM2	23

/*
 * Character macros
 */
#define iswhite(x) ((x) == '\n' || (x) == '\t' || (x) == ' ')
#define toupper(x) ((x) >= 'a' && (x) <= 'z' ? (x) & 0xdf : (x))
#define ctoi(x) ((x) - 0x30)

/*
 * Debugging macro
 */
#define DBG(p) { if (sp_debug) {p} }
