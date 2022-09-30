/* @(#)key.c	1.1 - test RAW keyboard input */
#include <sys/types.h>
#include <termio.h>
#include <stdio.h>
#include "sys/kd.h"

static struct termio tty_params; /* save area for resetting the tty params */

main ()
{
    unsigned char c;

    tty_setparms ();
    if (ioctl (0, KDSKBMODE, K_RAW) < 0) {
	perror ("KDSKBMODE: K_RAW");
	exit (1);
    }
    if (ioctl (0, KDGKBMODE, 0) != K_RAW)
	fprintf (stderr, "KDGKBMODE != K_RAW\n");

    printf ("Keyboard in RAW mode, use SYS REQ to exit this program\n");
    while (read (0, &c, 1) == 1) {
	printf ("0x%02x ", c);
	c &= 0x7F;
	if (c == 0x54)	/* stop on SYSREQ */
	    break;
	if (c < ' ') {
	    printf ("^");
	    c += '@';
	}
	printf ("%c\n", c);
	fflush (stdout);
    }
    if (ioctl (0, KDSKBMODE, K_XLATE) < 0) {
	perror ("KDSKBMODE: K_XLATE");
	exit (1);
    }
    if (ioctl (0, KDGKBMODE, 0) != K_XLATE)
	fprintf (stderr, "KDGKBMODE != K_XLATE\n");
    tty_resetparms ();
    printf ("\n");
}


/* set the tty parameters */
tty_setparms ()
{
    struct termio tty_set;

    /* get the current parameters */
    if (ioctl (0, TCGETA, &tty_params) < 0) {
	perror ("tty_setparms: ioctl-get");
	exit (1);
    }
    /* copy them to a temp buffer */
    tty_set = tty_params;

    /* set the new parameters */
    /* (set individually for easier changing for the next usage!) */
    tty_set.c_iflag &= ~(INPCK | ISTRIP | INLCR | ICRNL | IXON | IXANY | IXOFF);
    tty_set.c_iflag |=  (IGNBRK);
    tty_set.c_cflag  =  (tty_set.c_cflag & ~CSIZE) | CS8;
    tty_set.c_cflag |=  (CREAD  | CLOCAL | PARODD | PARENB);
/*  tty_set.c_cflag &= ~(PARENB | HUPCL); */
    tty_set.c_cflag &= ~(HUPCL);
    tty_set.c_lflag &= ~(ICANON | ECHO | ISIG | NOFLSH);
    tty_set.c_cc [VEOF] = 1;
    tty_set.c_cc [VEOL] = 1;

    /* update tty */
    if (ioctl (0, TCSETA, &tty_set) < 0) {
	perror ("tty_setparms: ioctl-set");
	exit (1);
    }
}

tty_resetparms ()
{
    /* restore previous tty params */
    if (ioctl (0, TCSETA, &tty_params) < 0)
	perror ("tty_resetparms: ioctl-set"), exit (1);
}
