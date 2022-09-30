static char *uportid = "@(#)setkey.c	Microport Rev Id  1.3.8 11/24/86";
/*
 * @(#)tstkey.c	1.2
 * Setkey program
 * usage: setkey code string
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
    setkey (1, K_NORMTAB,  NORMKEY | 0x1b);
    setkey (1, K_SHIFTTAB, NORMKEY | 0x1b);
    setkey (41,  K_NORMTAB,  NORMKEY | '`');
    setkey (41,  K_SHIFTTAB, NORMKEY | '~');
}

setkey (index, table, value)
    int index, table, value;
{
    struct kbentry ke;

    ke.kb_index = index;
    ke.kb_table = table;
    ke.kb_value = value;

    if (ioctl (0, KDSKBENT, &ke)) {
	perror ("ioctl");
	exit (2);
    }
}
