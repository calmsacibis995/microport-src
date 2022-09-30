static char *uportid = "@(#)atspec.c	Microport Rev Id 1.3.3  6/18/86";
/* AT specific routines
**
**	includes functions to
**	 read the motherboard switches
**	 beep the speaker
**	 reset the system
**	 preform the kernel debugging
**	M000: lance 4-3-86
**		  Change to fiddle a uadmin(A_SHUTDOWN, AD_BOOT) for CTRL-ALT-DEL
**  M001: lance 5-4-86
**		  Change switch read to remove extraneous junk.
*/


#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/sysinfo.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/uadmin.h"

#define CMOS_PORT 0x70		/* port address for reading the switch */
#define DIAG_STAT 0x8E		/* command to read the switch */
#define C_EQUIP 0x94
#define TONE_DUR 30 		/* how long to sound the beeper */
#define HALF_TONE_1 300   	/* half tone duration */
#define HALF_TONE_2 100	  	/* second half tone duration */
#define SPKR_PORT 0x61		/* keyboard sense port */

extern unsigned char getchar();
int	eqstat, eqsw, eqout;

/*
** geteqsw
**     read the physical equipment switch on the motherboard
*/
geteqsw()	 /* get the setings of the physical equipment switch */
{
	unsigned char tmp, data;
	unsigned timer;

	/* M001 */
	outb( CMOS_PORT, C_EQUIP);
	tmp = inb( CMOS_PORT + 1 );
	return (tmp);
}

/*
** beep
**	beep the speaker with an unpleasant tone 
*/
beep(){
	unsigned char  pctrl, prest;
	int tonedur,hlfton1,hlfton2;
	tonedur = TONE_DUR;
	pctrl = prest = inb( SPKR_PORT );
	while ( tonedur-- ) {
		pctrl &= 0xFC;
		outb( SPKR_PORT, pctrl);
		hlfton1 = HALF_TONE_1;
		while ( hlfton1-- ) {}
		pctrl |= 2;
		outb( SPKR_PORT, pctrl);
		hlfton2 = HALF_TONE_2;
		while ( hlfton2-- ) {}
	}
	outb( SPKR_PORT,prest );
	return;
}

int	dieflag = 0;	/* check by idle loop in slp.c:sched */
time_t  startsd = 0;	/* lbolt time when first control-alt-del hit */
#define CADWINDOW 60*10	/* approximately 10 seconds	*/

/* kerndebug
**	routine to reset the AT or enter debugging mode
**	handles the control-alt-delete sequence to allow rebooting.
*/

int rebootcount = 0;		/* # times control-alt-del has been hit */

kerndebug() {
	extern	ualock, lbolt;

	if (ualock == 0) {				/* we can do a kadmin */
		startsd = lbolt;
		ualock = 2;
		dieflag = 1;
		return;
	}
	/* already in shutdown. if this is from another control-alt-del
	 * and is less than CADWINDOW ticks, return. this will prevent
	 * keyboard debounces, as well as rapid c-a-d's from causing
	 * a reboot (and thus possibly damaging filesystems).
	 */

	if (rebootcount++ < 5) {
		if ((startsd != 0) && (lbolt - startsd < CADWINDOW)) {
			printf("The system is being shut down\n");
			return;
		}
	}
	mdboot(AD_BOOT, 0);
}
	
