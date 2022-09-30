/*
 * Speaker device driver - Marc de Groot
 */

/*
 * Include files
 */
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include "spkr.h"

/*
 * Global variables
 */
int spkr_open = 0;			/* spkr_open != 0 means dev is open */
int sp_divisor;				/* Freq divisor. Loaded into timer. */
int sp_note_name;			/* ASCII name of the current note.  */
int sp_ticks;				/* # of ticks that the tone lasts.  */
int sp_tempo;				/* # of ticks in a whole note.	    */
int sp_octave;				/* How many times we dhift divisor. */
int sp_symbol_type;			/* Next sym. state mach. will parse */
int sp_debug = 0;			/* sp_debug != 0 means print diags  */
					/* sp_debug may be changed with	    */
					/* /etc/patch			    */

long sp_divisors[] =			/* The chromatic scale.	*/
{
	61856,				/* A				*/
	58384,				/* A#				*/
	58384,				/* Bb				*/
	55107,				/* B				*/
	52014,				/* C				*/
	49095,				/* C#				*/
	49095,				/* Db				*/
	46340,				/* D				*/
	43739,				/* D#				*/
	43739,				/* Eb				*/
	41284,				/* E				*/
	38966,				/* F				*/
	36779,				/* F#				*/
	36779,				/* Gb				*/
	34715,				/* G				*/
	32767,				/* G#				*/
	32767				/* Ab				*/
};

int sp_note_names[] =
{
	'A' << 8,
	'A' << 8 | '#',
	'B' << 8 | 'b',
	'B' << 8 ,
	'C' << 8,
	'C' << 8 | '#',
	'D' << 8 | 'b',
	'D' << 8,
	'D' << 8 | '#',
	'E' << 8 | 'b',
	'E' << 8,
	'F' << 8,
	'F' << 8 | '#',
	'G' << 8 | 'b',
	'G' << 8,
	'G' << 8 | '#',
	'A' << 8 | 'b'
};

/*
 * spopen - Open the speaker device 
 */
spopen(dev, flag)
int dev, flag;
{
	if(spkr_open)			/* If spkr already open		*/
	{
		u.u_error = ENXIO;	/* Report error			*/
		return;
	}
	spkr_open = 1;			/* Others can't use spkr	*/
	sp_tempo = 20;			/* Whole note = 1/3 second	*/
	sp_symbol_type = SP_OCTAVE;	/* Initialize state machine	*/
	sp_octave = 4;			/* Sane value for the octave	*/
	sp_note_name = 'A' << 8;	/* Sane note name		*/
	sp_ticks = 10;			/* Sane # of ticks ( 1/6 sec )	*/
}

/*
 * spclose - close the speaker device
 */
spclose(dev)
int dev;
{
	if(!spkr_open)
	{
		u.u_error = ENXIO;
		return;
	}
	spkr_open = 0;
}

/*
 * spread - speaker read routine
 */
spread(dev)
int dev;
{
}

/*
 * spwrite - speaker write routine
 *
 * Description of character output:
 * The spwrite routine incorporates a simple state machine which
 * interprets a representation of musical notes.  A whitespace
 * character causes a reset of the state machine to its initial
 * state, and plays the note set by the previous non-whitespace
 * character string. That character string is expected to consist
 * of:	odnn
 * where:	o is the octave number
 *		d is the duration of the note; one of
 *			w for whole note
 *			h for half note
 *			q for quarter note
 *			e for eighth note
 *			s for sixteenth note
 *			t for thirty-secondth note
 *			(Upper or lower case)
 *		nn is a one- or two-character string giving the note
 *		name. The first character is one of
 *			A B C D E F G  (Upper or lower case)
 *		The second character, if present, is one of
 *			b #  (To indicate flat or sharp, respectively)
 *
 * Example:
 * The string "3qbb 5wf " would play a B-flat quarter note in the third
 * octave, and then an F-natural whole note in the fifth octave.
 * 
 * Bugs:
 * There is no detection of erroneous note representations. Unexpected
 * characters will produce unpredictable results.
 */
spwrite(dev)
int dev;
{
	char c;
	unsigned int i;
	for(i = u.u_count; i--; u.u_base++)
	{
		copyin(u.u_base, &c, 1);
		state_machine(c);
	}
	u.u_count = 0;
}

/*
 * spioctl - speaker ioctl routine
 *
 * Functions supported:
 *
 * SP_SET_DIVISOR - Set the frequency divisor. The speaker output
 * frequency will be 1193180 / iarg Hertz.
 *
 * SP_TURN_ON - Turn on the loudspeaker for a certain number of
 * ticks of the system clock.
 *
 * SP_SET_FREQUENCY - The frequency divisor is set such that the
 * speaker output will be iarg Hertz the next time it is turned on.
 * This frequency is independent of the processor frequency.
 *
 * SP_SET_TICKS - Set the number of ticks of the system clock. The
 * speaker will be turned off after this number of ticks the next time
 * it is turned on.
 *
 */

spioctl(dev, cmd, arg)
int dev, cmd;
union ioctl_arg arg;
{
	switch(cmd)
	{
		case SP_SET_DIVISOR:
		{
			sp_divisor = arg.iarg;
			break;
		}			

		case SP_TURN_ON:
		{
			sp_setparms();
			sp_turn_on();
			break;
		}

		case SP_SET_FREQUENCY:
		{
			sp_divisor = 1193180L / (long)arg.iarg;
			break;
		}

		case SP_SET_TICKS:
		{
			sp_ticks = arg.iarg;
			break;
		}
	}
}

/*
 * Supporting subroutines follow
 */

/*
 * sp_setparms - Set up the 8253 timer for sound
 */
sp_setparms()
{
	outb(SP_TIMER + 3, 0xb6);	/* Timer 2, lsb, msb, binary */
	outb(SP_TIMER + 2, sp_divisor & 0xff);	/* Output lo byte    */
	outb(SP_TIMER + 2, sp_divisor >> 8 & 0xff); /* Output hi byte    */
}

/*
 * sp_turn_on() - Turn on the loudspeaker gate, delay for a number of
 * system ticks, and turn the loudspeaker back off.
 */
sp_turn_on()
{
	outb(SP_PORTB, inb(SP_PORTB) | 3);	/* Gate timer 2, turn sp on */
	delay(sp_ticks);			/* Wait sp_ticks/HZ seconds */
	outb(SP_PORTB, inb(SP_PORTB) & ~3);	/* Gate timer 2, turn sp off*/
}

/*
 * state_machine - The note-playing state machine
 */
state_machine(c)
char c;
{
	DBG(printf("state_machine(0x%x) ", c);)
	if(iswhite(c))			/* Whitespace inits state machine */
	{
		if(sp_symbol_type != SP_OCTAVE)
		{
			sp_cnvt_note();	/* Cnvrt sp_note_name to sp_divisor */
			sp_setparms();	/* Load sp_divisor into timer hdwe. */
			sp_turn_on();   /* Load timer and turn on speaker */
		}
		sp_symbol_type = SP_OCTAVE; /* Get set up for next note */
		DBG(printf("EXIT1 ");)
		return;
	}
	DBG(printf("sp_symbol_type=%d ", sp_symbol_type);)
	switch(sp_symbol_type)
	{
		case SP_OCTAVE:
			sp_octave = ctoi(c);
			sp_symbol_type = SP_NOTE_LEN;
			DBG(printf("sp_octave=%d ",sp_octave);)
			break;

		case SP_NOTE_LEN:
			sp_symbol_type = SP_NOTENM1;
			switch(c)
			{
				case 'W':
				case 'w':
					sp_ticks = sp_tempo;
					break;
				case 'H':
				case 'h':
					sp_ticks = sp_tempo / 2;
					break;
				case 'Q':
				case 'q':
					sp_ticks = sp_tempo / 4;
					break;
				case 'E':
				case 'e':
					sp_ticks = sp_tempo / 8;
					break;
				case 'S':
				case 's':
					sp_ticks = sp_tempo / 16;
					break;
				case 'T':
				case 't':
					sp_ticks = sp_tempo / 32;
					break;
			}
			DBG(printf("sp_ticks=%d ", sp_ticks);)
			break;
		case SP_NOTENM1:
			sp_note_name = toupper(c) << 8;	
			sp_symbol_type = SP_NOTENM2;
			DBG(printf("1st:sp_note_name=0x%x ",sp_note_name);)
			break;
		case SP_NOTENM2:
			if(c == '#' || c == 'b')
			{
				sp_note_name |= c;
			}
			DBG(printf("2nd:sp_note_name=%d ",sp_note_name);)
			break;
	}
	DBG(printf("EXIT2 ");)
}

/*
 * sp_cnvt_note - Given a note designator, convert to a frequency
 *		  divisor and a number of ticks.
 */
sp_cnvt_note()
{
	int i;
	DBG(printf("sp_cnvt_note() ");)
	for(i = 0; i < 17; i++)
	{
		if(sp_note_names[i] == sp_note_name)
		{
			sp_divisor = sp_divisors[i] >> sp_octave;
			DBG(printf("sp_divisor=%d, EXIT1 ", sp_divisor);)
			return;
		}
	}
	DBG(printf("EXIT2 ");)
}
