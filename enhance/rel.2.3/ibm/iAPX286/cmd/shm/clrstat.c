/* clrstat.c - mouse input */
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>

static char *vidram_get();

main ()
{
    clear ((key_t)  0xb8000, 32768, 0);
}

clear ( key, len, flags)
    key_t key;
    unsigned len;
    int flags;
{
    unsigned short *vidram;
    char *vidram_get();
    int i;

    vidram = (unsigned short *) vidram_get (key, len, flags);

    for (i = 24*80; i < 25*80; i++)
	vidram [i] = 0x0741;
}

static char *
vidram_get (key, len, flags)
    key_t key;
    unsigned len;
    int flags;
{
    char *vidram, *shmat();
    int shmid;
    extern int errno;

    if ((shmid = shmget (key, len, flags)) == -1) {
	if (errno == EEXIST) {
	    fprintf (stderr, "Segmemt already exists\n");
	    return NULL;
	}
	perror ("shmget");
	fprintf (stderr,
	    "shmget (key=0x%lx, len=0x%x, flags=0x%x)\n", key, len, flags);
	exit (2);
    }
    if ((vidram = shmat (shmid, (char *) 0, 0)) == (char *) -1)
	perror ("shmat"), exit (2);

    return vidram;
}
