/* shmcreate.c - create shared physical address segments */
#include <sys/types.h>
#include <sys/ipc.h>
#include "shm.h"
#include <stdio.h>

static char *sccs_id = "@(#)shmcreate.c	1.6";

main (argc, argv)
    int	argc;
    char **argv;
{
	key_t key;
	long len;
	unsigned shmlen;
	extern errno;
	int shmid, shmget (), i, *ip;
	char *vidram, *shmat (), *addr;
	unsigned char verbose = 0, overly_verbose = 0;
	struct shmid_ds shmem_status;
	ushort newmode = 0777;
	long strtol();
	int c;
	extern char *optarg;
	extern int optind, opterr;

	while ((c = getopt (argc, argv, "vVp:")) != EOF)
	    switch (c) {
		case 'v': verbose = 1;
			  break;
		case 'V': overly_verbose = 1;
			  verbose = 1;
			  break;
		case 'p': newmode = (ushort) strtol (optarg, (char **) 0, 8);
			  break;
	    }

	if ((argc - optind) < 3)
	    usage (*argv);

	key  = (key_t)  strtol (argv [optind++], (char **) 0,  0);
	addr = (char *) strtol (argv [optind++], (char **) 0, 16);
	len  = strtol (argv [optind++], (char **) 0,  0);

	while (len > 0) {
	    errno = 0;
	    if (verbose) {
		printf ("Attaching key %08lx to %08lx len %08lx mode "
		    ,(key_t) key
		    ,(char *) addr
		    ,len > 65535 ? (long) 65535 : (long) len
		);
		printperm (newmode & 0666, 3);
	    }

    retry:
	    if (len > 65535)
		shmlen = 65535;
	    else
		shmlen = len;

	    if ((shmid = shmget (	/* *special* shmget call */
		    (key_t) key
		    ,shmlen
		    ,IPC_CREAT | IPC_SHMPHYS
		    ,(char *) addr
		)) == -1
	    ) {
		perror ("Shmget");
		exit (1);
	    }
	    if (verbose)
		printf (" id: %d\n", shmid);

	    if (shmctl (shmid, IPC_STAT, &shmem_status) == -1) {
		perror ("Shmctl-status");
		exit (1);
	    }
		
	    if (shmem_status.shm_nattch) {
		fprintf (stderr,
		    "Segment attached to another process, mode not changed.\n");
		exit (3);
	    }
	    if (shmem_status.shm_cpid != getpid ()) {
		if (verbose)
		    fprintf (stderr,
			"Segment exists - removing and reallocating.\n");
		if (shmctl (shmid, IPC_RMID, (char *) 0) == -1) {
		    perror ("Shmctl-remove");
		    exit (1);
		}
		goto retry;
	    }
	    shmem_status.shm_perm.mode = newmode;	/* set mode*/
	    if (shmctl (shmid, IPC_SET, &shmem_status) == -1) {
		perror ("Shmctl-set");
		exit (1);
	    }
	    if (shmctl (shmid, IPC_STAT, &shmem_status) == -1) {
		perror ("Shmctl-status");
		exit (1);
	    }
	    if (overly_verbose) {
		printf ("uid=%u gid=%u cuid=%d cgid=%d mode=%o seq=%d key=%d\n"
		    ,shmem_status.shm_perm.uid
		    ,shmem_status.shm_perm.gid
		    ,shmem_status.shm_perm.cuid
		    ,shmem_status.shm_perm.cgid
		    ,shmem_status.shm_perm.mode
		    ,shmem_status.shm_perm.seq
		    ,shmem_status.shm_perm.key
		);
		printf("segsz=%lx segpcc=%lx lpid=%d cpid=%d nattch=%d cnattch=%d\n"
		    ,shmem_status.shm_segsz
		    ,shmem_status.shm_segpcc
		    ,shmem_status.shm_lpid
		    ,shmem_status.shm_cpid
		    ,shmem_status.shm_nattch
		    ,shmem_status.shm_cnattch
		);
		printf ("atime=%lx dtime=%lx ctime=%lx\n"
		    ,shmem_status.shm_atime
		    ,shmem_status.shm_dtime
		    ,shmem_status.shm_ctime
		);
	    }
	    len  -= shmlen;		/* do next segment */
	    addr += 65536L;
	    key ++;
	}
	exit (0);
}

usage (s)
    char *s;
{
    fprintf (stderr, "Usage: %s [-vV] [-p mode] key adrs len\n", s);
    fprintf (stderr, "where:\n\t-v\tverbose\n");
    fprintf (stderr, "\t-V\toverly_verbose\n");
    fprintf (stderr, "\t-p perm\t(octal) permission (default: 666)\n");
    fprintf (stderr, "\tkey\t(decimal) key (or 0xHEX)\n");
    fprintf (stderr, "\tadrs\t(hex) physical address\n");
    fprintf (stderr, "\tlen\t(unsigned decimal) length\n");
    exit (1);
}

printperm (mode, len)
    int mode;
    int len;
{
    int i;
    int bits;

    for (i = (len - 1) * 3; i >= 0; i -= 3) {
	bits = mode >> i;
	printf ("%c%c%c"
	    ,bits & 4 ? 'r' : '-'
	    ,bits & 2 ? 'w' : '-'
	    ,bits & 1 ? 'x' : '-'
	);
    }
}
