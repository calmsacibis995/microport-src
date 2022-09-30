static char *uportid = "@(#)swapkey.c	Microport Rev Id  1.3.8 11/24/86";
/*
 * @(#)swapkey.c	1.2
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/kd.h>
#include <sys/kd_info.h>
#include <sys/setkey.h>


main (argc, argv)
    int  argc;
    char **argv;
{
    int key1, key2;
    long strtol ();

    if (--argc > 0) {
	key1 = strtol (*++argv, 0, (char **) 0);
	if (--argc > 0) {
	    key2 = strtol (*++argv, 0, (char **) 0);
	    swapkey (key1, K_NORMTAB,	   key2, K_NORMTAB);
	    swapkey (key1, K_SHIFTTAB,	   key2, K_SHIFTTAB);
	    swapkey (key1, K_ALTTAB,	   key2, K_ALTTAB);
	    swapkey (key1, K_ALTSHIFTTAB,  key2, K_ALTSHIFTTAB);
	    exit (0);
	}
    }
    fprintf (stderr, "swapkey key1 key2\n");
    exit (1);

#ifdef	OLD_WAY_2
    /* swap ctrl and caps lock */
    swapkey (0x1d, K_NORMTAB,	   0x3a, K_NORMTAB);
    swapkey (0x1d, K_SHIFTTAB,	   0x3a, K_SHIFTTAB);
    swapkey (0x1d, K_ALTTAB,	   0x3a, K_ALTTAB);
    swapkey (0x1d, K_ALTSHIFTTAB,  0x3a, K_ALTSHIFTTAB);
#endif

#ifdef	OLD_WAY
    setkey (0x1d, K_NORMTAB,	 SHIFTKEY | CAPS_LOCK);
    setkey (0x1d, K_SHIFTTAB,	 SHIFTKEY | CAPS_LOCK);
    setkey (0x1d, K_ALTTAB,	 SHIFTKEY | CAPS_LOCK);
    setkey (0x1d, K_ALTSHIFTTAB, SHIFTKEY | CAPS_LOCK);
    setkey (0x3a, K_NORMTAB,	 SHIFTKEY | LEFT_CTRL);
    setkey (0x3a, K_SHIFTTAB,	 SHIFTKEY | LEFT_CTRL);
    setkey (0x3a, K_ALTTAB,	 SHIFTKEY | LEFT_CTRL);
    setkey (0x3a, K_ALTSHIFTTAB, SHIFTKEY | LEFT_CTRL);
#endif
}

swapkey (key1, table1, key2, table2)
    int key1, table1, key2, table2;
{
    int value1 = getkey (key1, table1);
    int value2 = getkey (key2, table2);
    setkey (key1, table1, value2);
    setkey (key2, table2, value1);
}

getkey (index, table)
    int index, table;
{
    struct kbentry ke;

    ke.kb_index = index;
    ke.kb_table = table;

    if (ioctl (0, KDGKBENT, &ke)) {
	perror ("get ioctl");
	exit (2);
    }
    return ke.kb_value;
}

setkey (index, table, value)
    int index, table, value;
{
    struct kbentry ke;

    ke.kb_index = index;
    ke.kb_table = table;
    ke.kb_value = value;

    if (ioctl (0, KDSKBENT, &ke)) {
	perror ("set ioctl");
	exit (2);
    }
}
