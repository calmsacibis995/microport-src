static char *uportid = "@(#)setkey.c	Microport Rev Id  1.3.8 11/24/86";
/*
 * @(#)prtkey.c	1.3
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


char *table [] = {
    "K_NORMTAB",
    "K_SHIFTTAB",
    "K_ALTTAB",
    "K_ALTSHIFTTAB",
    "K_CTRLTAB"
};
char *prefix [] = {
    "NORMKEY",
    "SHIFTKEY",
    "BREAKKEY",
    "SS2PFX",
    "SS3PFX",
    "CSIPFX",
    "CS2PFX",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "?",
    "DOSKEY",
    "NOKEY"
};

main (argc, argv)
    int  argc;
    char **argv;
{
    struct kbentry ke;
    char c;

    for (ke.kb_index = 0; ke.kb_index < 128; ke.kb_index++) {
	for (ke.kb_table = 0; ke.kb_table < 5; ke.kb_table++) {
	    if (ioctl (0, KDGKBENT, &ke) == -1) {
		fprintf (stderr, "%d %d ", ke.kb_index, ke.kb_table);
		perror ("ioctl");
		exit (2);
	    }
	    printf ("%d\t%s\t", ke.kb_index, table [ke.kb_table]);
	    if (ke.kb_value & NUMLCK) printf ("NUMLCK | "); 
	    if (ke.kb_value & CAPLCK) printf ("CAPLCK | "); 
	    if (ke.kb_value & CTLKEY) printf ("CTLKEY | "); 
	    printf ("%s | ", prefix [(ke.kb_value & TYPEMASK) >> 8]);

	    c = ke.kb_value & 0xFF;
	    if (c < 0x20)
		printf ("0x%x\n", c);
	    else
		printf ("%c\n", c);
	}
    }
}
