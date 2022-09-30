/*	@(#)	1.1	*/
/*
 *  strm.c - contains routines to implement strm(1) - a streaming i/o utility 
 * Modification History
 *	M000 uport!dwight	Mon May 4 1987
 *	change parent handling of semaphores, fixes parent hanging problem
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/stat.h>

/* debug output goes through a common output routine debugout() */
#undef DEBUG
#define DEBUG
  
#ifdef DEBUG
/* complete semaphore operation trace can be obtained with SEMDEBUG */
#define SEMDEBUG

/* debug output can be directed to a file if desired */
#undef DEBFILE
/*#define DEBFILE*/
int strmdbg = 0;

#define	DBG0(x) if (strmdbg&(1<<0)) { x; } /* reserved */
#define	DBG1(x) if (strmdbg&(1<<1)) { x; } /* write and read op progress */
#define	DBG2(x) if (strmdbg&(1<<2)) { x; } /* read/write process entry/exit */
#define	DBG3(x) if (strmdbg&(1<<3)) { x; } /* process completion (strmdone) */
#define	DBG4(x) if (strmdbg&(1<<4)) { x; } /* multiple volume debug */
#define	DBG5(x) if (strmdbg&(1<<5)) { x; } /* sem op exit/entry progress */
#define	DBG6(x) if (strmdbg&(1<<6)) { x; } /* trace reads less than Bufsize */
#define	DBG7(x) if (strmdbg&(1<<7)) { x; } /* reserved */
#else NOT DEBUG
#define	DBG0(x) { }
#define	DBG1(x) { }
#define	DBG2(x) { }
#define	DBG3(x) { }
#define	DBG4(x) { }
#define	DBG5(x) { }
#define	DBG6(x) { }
#define	DBG7(x) { }
#endif DEBUG

/* global externs from the outside world */
extern	errno;

/* external library function calls declared */
extern unsigned char	*shmat();

/* the Maximum number of device processes available */
#define MAXPROCS	3
int numbufs = MAXPROCS;		/* patchable number of buffers to alloc */

/* misc defs */
#define	UCPERROR (unsigned char *) -1
#define	UERROR (unsigned int) -1
#define	IERROR (int) -1
#define	CPERROR (char *) -1
union { long l; short s[2]; char c[4]; } U;

/* each process controls a shared memory buffer keyed by the process number */
int	strm_shmid[MAXPROCS];

/* process numbers rotate from 0 to numprocs-1 to 0 */
/* backwards to previous process */
#define PREV(this) ((this+(numprocs-1))%(numprocs))
/* and forwards to next process */
#define NEXT(this) ((this+(1))%(numprocs))

/*
 * SEMAPHORE Operations and types 
 */
#define NUMSEMS 1

/*
 * Define a different operation structure for each different op 
 */

/* a V_ semaphore operation increments semval */
struct 	sembuf vsops[NUMSEMS];

/* a GET semaphore blocks until semval is zero */
struct 	sembuf gsops[NUMSEMS];

/* a P_ semaphore operation decrements semval (will block if zero) */
struct 	sembuf psops[NUMSEMS];

/*
 * Define all operations for each type of semaphore 
 */

/* protect empty buffer from being emptied */
#define EMPTYSEM 0
#define V_EMPTYSEM(id)	(strmsem(EMPTYSEM,id,vsops))
#define GETEMPTYSEM(id)	(strmsem(EMPTYSEM,id,gsops))
#define P_EMPTYSEM(id)	(strmsem(EMPTYSEM,id,psops))

/* protect full buffer from being filled */
#define FULLSEM (EMPTYSEM+1)
#define V_FULLSEM(id)	(strmsem(FULLSEM,id,vsops))
#define GETFULLSEM(id)	(strmsem(FULLSEM,id,gsops))
#define P_FULLSEM(id)	(strmsem(FULLSEM,id,psops))

/*the previous process semaphore keeps instance 2 from queing writes */
/* before instance 1, etc. */
#define PREVSEM (FULLSEM+1)
/* actually you must V_ the PREV() id after GETing it */
#define V_PREVSEM(id)	(strmsem(PREVSEM,(id),vsops))
#define GETPREVSEM(id)	(strmsem(PREVSEM,(id),gsops))
#define P_PREVSEM(id)	(strmsem(PREVSEM,(id),psops))

#define NUMTYPES (PREVSEM+1)

/* each semaphore has a unique ID stored in */
int	strm_semid[NUMTYPES][MAXPROCS];

int	semval;	/* last value of last op */

/* option can be "-i", "-o", or unspecified if not ambiguous */
#define NOPTION	0		/* no option specified, check for pipe */
#define IN	1		/* copy in from specified device */
#define OUT	2		/* copy out to specified device */
int	Input = 0,		/* Input defaults to stdin (fd=0) */
	Output = 1;		/* Output defaults to stdout (fd=1) */

/* buffer size and count */
#define BLOCKSIZE 512		
#define NBLOCKS 120		/* default number of blocks of BLOCKSIZE */
unsigned Bufsize = NBLOCKS*BLOCKSIZE;	/* default record size */

/* an array of buffer pointers per process */
unsigned char 	*BDbuf[MAXPROCS];

/* an array of buffer size pointers per process */
unsigned int 	*BDsize[MAXPROCS];

/* number of procs actually init()ed */
int	numprocs=0;

/*	
    strm - a streaming utility to be used in combination with an
    i/o utility such as cpio(1) or tar(1)

    Functional Description:

    strm gathers a stream of data from stdin and writes large blocks
    to the specified device, or reads from the specified device and
    outputs a stream to stdout.

    strm decides whether to read or write from the specified device
    based on which of stdin or stdout is a pipe.  The flags -i & -o
    are provided to remove any ambiguity.

    For the case of OUTPUT to a device, strm fork()s a process soley
    for reading from standard input, this process fills three buffers
    and fork()s the same number of write processes, each with its own
    shared memory buffer.  These processes queue their write requests
    to the specified device one after another, such that at least one
    write request is always queued, one is being written, and one of 
    the buffers is being read into.

    For the INPUT case, a single write process streams out to stdout,
    and three read requests are queued as before.

*/
char devicename[32];	/* to hold the path name of specified device */
int childPID[MAXPROCS];
int childSTAT[MAXPROCS];
static strmdone();
static strmrdone();
unsigned char *strmshmget();

main(argc, argv)
char **argv;
{
    register int Option=NOPTION;
    register int pid;
    register int numtypes;
    char *parmptr;
    int retval=2;
    int waitret=0;
    int i=0;

    /* must specify a device at least */
    if((argc <= 1))
	usage();

    /* save pointer to device name */
    parmptr=argv[1];

    if((int)signal(SIGQUIT,strmdone)==-1)
	{
	strm_error(" Can't catch SIGQUIT \n");
	}
    if((int)signal(SIGBUS,strmdone)==-1)
	{
	strm_error(" Can't catch SIGBUS \n");
	}
    if((int)signal(SIGSEGV,strmdone)==-1)
	{
	strm_error(" Can't catch SIGSEGV \n");
	}
    if((int)signal(SIGSYS,strmdone)==-1)
	{
	strm_error(" Can't catch SIGSYS \n");
	}
    if((int)signal(SIGPWR,strmdone)==-1)
	{
	strm_error(" Can't catch SIGPWR \n");
	}
    if((int)signal(SIGHUP,SIG_IGN)==-1)
	{
	strm_error(" Can't catch SIGHUP \n");
	}

    /* now initialize put and get semaphore operation structures */
    vsops[0].sem_num = 0;
    vsops[0].sem_op = 1;	/* 1 = Increment semval */
    vsops[0].sem_flg = 0666;
    gsops[0].sem_num = 0;
    gsops[0].sem_op = 0;	/* 0 = wait if non zero */
    gsops[0].sem_flg = 0666;
    psops[0].sem_num = 0;
    psops[0].sem_op = -1;	/* -1 = Decrement semval or wait if zero */
    psops[0].sem_flg = 0666;

    /* parse the argument list if any */
    while(*++argv[1]) 
	{
	switch(*argv[1]) 
	    {
	    /* explicit input option specified */
	    case 'i':
		if((argc <= 2))
		    usage();
		else
		    parmptr=argv[2];
		Option = IN;
		break;

	    /* explicit output option specified */
	    case 'o':
		if((argc <= 2))
		    usage();
		else
		    parmptr=argv[2];
		Option = OUT;
		break;

	    /* must be a device name or bad option */
	    default:
		if(!Option && argc != 2)
		    usage();
	    }
	} /* end argument parsing */

    /* if no specification */
    if(!Option) 
	{
	/* determine from stat()s the most logical configuration */
	if (isapipe(Input))
	    {
	    if (isapipe(Output))
		{
		fprintf(stderr,"Output and Input are both pipes\n");
		fprintf(stderr,"Options must include either -i or -o\n");
		exit(2);
		}
	    Option = OUT;
	    }
	else
	    {
	    if (!isapipe(Output))
		{
		fprintf(stderr,"Neither Output and Input are pipes\n");
		fprintf(stderr,"Options must include either -i or -o\n");
		exit(2);
		}
	    Option = IN;
	    }
	} /* end auto option specification */

    debugopen();

    do /*get a shared memory buffer and set of semaphores for each process*/
	{
	if (!(BDbuf[numprocs]=strmshmget((key_t)(numprocs+1)
	 ,Bufsize+sizeof(unsigned int),IPC_CREAT|0666)))
	    {
	    break;
	    }
	BDsize[numprocs]=(unsigned int *)BDbuf[numprocs];/*size, then buf*/
	BDbuf[numprocs]+=sizeof(unsigned int);/*point to buffer(past size)*/

	for (numtypes=0;numtypes<NUMTYPES;numtypes++)
	    {
	    if ((strm_semid[numtypes][numprocs] =
	     semget ((key_t)((numtypes*numbufs)+numprocs+1)
	     , 1, IPC_CREAT|0666)) == IERROR)
		{
		strm_error("cannot get a semaphore for %ld \n"
		 ,(key_t)numprocs);
		}
	    /* must initialize all semaphores to non-zero value before any P */
	    if ( semctl (strm_semid[numtypes][numprocs] 
	     , 0, SETVAL, 1) == IERROR)
		{
		strm_error("cannot set a semaphore for %ld \n"
		 ,(key_t)numprocs);
		}
	    }
	numprocs++;
	}
    while (numprocs < numbufs);
    /* while more processes to init() */

    if (!numprocs)
	{
	fprintf(stderr,"Could not get any shared memory buffers\n");
	fprintf(stderr
	 ,"you won't see any benefit from strm given the lack of memory\n");
	goto mainexit;	/* just set a break point there for exit cond */
	}

    /* depending on configuration either OUT to device or IN from it */
    switch(Option) 
	{
	case OUT:		
	    /* one option plus devicename max */
	    if(argc > 3)
		usage();
	    if(access(parmptr, 02) == -1) 
		{
		strm_error("strm: no write access to <%s>\n", parmptr);
		}
	    if((Output = open(parmptr, O_WRONLY)) < 0) 
		{
		strm_error("cannot open <%s> \n", parmptr);
		}
	    strcpy(devicename,parmptr);
	    /* catch interrupts and quit */
	    if((int)signal(SIGINT,strmrdone)==-1)
		{
		strm_error(" Can't catch SIGINT \n");
		}
	    i=0;
	    while(i<numprocs)
		{
		if(pid = fork()) /* the Parent */
		    {
		    childPID[i++] = pid;
		    }
		else if(pid == -1) 
		    {
		    strm_error("Cannot fork, try again\n");
		    }
		else 	/* the Child */
		    {
		    writedevproc(i);	
		    /* they exit(0) instead of return */
		    }
		}
	/*  5:00 PM 3/3/87
	    if((int)signal(SIGCLD,strmrdone)==-1)
	*/
	    if((int)signal(SIGCLD,SIG_IGN)==-1)
		{
		strm_error(" Can't catch SIGCLD \n");
		}
	    readinproc(childPID);
	    /* returns here because its the parent */
	    break;

	case IN:
	    /* one option plus devicename max */
	    if(argc > 3)
		usage();
	    if(access(parmptr, 04) == -1) 
		{
		strm_error("cannot read from <%s>\n", parmptr);
		}
	    if((Input = open(parmptr, O_RDONLY)) < 0) 
		{
		strm_error("cannot open <%s> \n", parmptr);
		}
	    strcpy(devicename,parmptr);

	    i=0;
	    while(i<numprocs)
		{
		if(pid = fork()) /* the Parent */
		    {
		    childPID[i++] = pid;
		    }
		else if(pid == -1) 
		    {
		    strm_error("Cannot fork, try again\n");
		    }
		else 	/* the Child */
		    {
		    readevproc(i);	
		    /* they exit(0) instead of return */
		    }
		} /* end fork() loop */

	    /* we won't know we're done until i/o utility closes its */
	    /* end of the pipe */
	    /*
	    if((int)signal(SIGCLD,strmdone)==-1)
		{
		strm_error(" Can't catch SIGCLD \n");
		}
	    */

	    if((int)signal(SIGPIPE,strmdone)==-1)
		{
		if (!isapipe(Output))
		    fprintf(stderr,"Output not a pipe, Can't do SIGPIPE \n");
		else
		    strm_error(" Can't do SIGPIPE \n");
		}

	    writeoutproc(pid);
#define WRITERMINATE
#ifdef WRITERMINATE
	    /* returns here because its the parent */
	    while (i--)
		{
		kill(childPID[i],SIGKILL);
		while((waitret=wait(&childSTAT[i])) != childPID[i])
		    {
		    if(waitret == IERROR)
			{
			switch (errno)
			    {
			    case EINTR:
				{
				/* got the signal and died */
				goto nextchild;
				/* no getum here, (quick way to break out)*/
				}
				break;

			    case ECHILD:
				{
				/* no more children, might as well die */
				retval = 0;
				goto mainexit;
				/* no getum here, (quick way to break out)*/
				}
				break;

			    default:
				goto nextchild;
				break;
			    }
			}
		    } /* end wait on child i */
nextchild:
		continue;
		} /* end child wait loop */
#endif WRITERMINATE
	    break;
	}	/* end Options switch */

    retval = 0;
    
    /* just set a break point here for exit condition */
mainexit:		/* please go through common exit of main */
			/* makes breakpoint on exit easier, etc. */
    strm_exit(retval);
}

readinproc(children)
int children[MAXPROCS];
{
    /* process is responsible for filling the three stream buffers */
    static status;
    int writeproc=0;
    unsigned int rv;
    unsigned int in=0;
    int done=0;

    DBG2(debugout("entering read in process\n"))
    P_PREVSEM(PREV(writeproc)); /* let the first one que it's write */
    do 
	{
	/* don't start filling a buffer that is not empty */
	GETEMPTYSEM(writeproc); /* wait until its zero */
	V_EMPTYSEM(writeproc); /* set it to one */

	/* fill the empty buffer */
	in = 0;
	DBG1(debugout("<Rp%ds%xQueued>\n",writeproc,Bufsize))
	while(!done && ((rv=(unsigned int)read(Input
	 , &(((unsigned char *)BDbuf[writeproc])[in])
	  , Bufsize - in)) != (Bufsize - in))) 
	    {
	    if(rv == 0) 
		{
		DBG1(debugout("in=%x,rv=%x,Bufsize=%x\n",in,rv,Bufsize))
		close(Input);
		done++;
		}
	    else if(rv == (unsigned int)-1) 
		{
		DBG1(debugout("read returned errno %d\n",errno))
		close(Input);
		done++;
		rv=0;
		}
	    DBG6(debugout("<r%x/t%x>",rv,in+rv))
	    in += rv;
	    rv=0;
	    }
	*BDsize[writeproc] = (in+rv);
	DBG1(debugout("<Rp%ds%xComplete>\n",writeproc,in+rv))
	/* protect the filling of the buffer */
	P_FULLSEM(writeproc);

	/* go on to next buffer/process */
	if(!done)
	    writeproc = NEXT(writeproc);

	}
    while (!done);

    GETEMPTYSEM(writeproc); /* wait until its zero */
#ifdef READTERMINATE
    V_EMPTYSEM(writeproc); /* and then set it to one */

    /* for each write process, wait for termination */
    done = numprocs;
    while(done)
	{
	DBG2(debugout("entering read wait on %d p%d\n",writeproc
	 ,children[writeproc]))
	while(wait(&status) != children[writeproc])
	    {
	    if (!status)
		break;
	    DBG2(debugout("process %d terminated\n",status))
	    }
	writeproc = NEXT(writeproc);
	done--;
	}
#endif READTERMINATE
    DBG2(debugout("exiting read in process\n"))
}

extern long lseek();

writedevproc(instance)
int instance;
{
/*process is responsible for emptying one of the three stream buffers*/
    unsigned bufsize;
    unsigned count;
    long ofilep, filep;	/* file pointer is the only way we know he's written */
    int pid;
    int done=0;

    /*instance now tells us which of the three processes 0, 1 or 2 we are*/
    DBG2(debugout("entering write dev process %d\n",instance))
    GETPREVSEM(PREV(instance));
    V_PREVSEM(PREV(instance)); 
    P_EMPTYSEM(instance); /* let the read routine fill us */
    P_PREVSEM(instance); /* let the next one wait on the FULL */
    while(!done) 
	{
	GETFULLSEM(instance); /* wait until the read routine fills us */
	V_FULLSEM(instance); /* protect our emptying of the buffer */
	if((bufsize = (unsigned int)*(BDsize[instance]))!=Bufsize)
	    {
	    done++;
	    }
	else
	    {
	    }
	if(!bufsize)
	    {
	    DBG2(debugout("exiting write dev process %d\n",instance))
	    (unsigned int)*(BDsize[NEXT(instance)])=0;
	    P_FULLSEM(NEXT(instance)); 
	    goto wdevexit;
	    }
	GETPREVSEM(PREV(instance)); /* once the previous write is done */
	V_PREVSEM(PREV(instance)); /* hold off next write */
again:
	ofilep = lseek (Output, (long)0, 1);	/* read original file pointer */
	DBG1(debugout("<old filep: %lx>\n",ofilep))
	DBG1(debugout("<Wp%ds%xQueued>\n",instance,bufsize))
	if((count=write(Output,BDbuf[instance],bufsize))!=bufsize) 
	    {
	    filep = lseek (Output, (long) 0, 1);	/* read file pointer */
	    DBG1(debugout("<filep: %lx>\n",filep))
	    DBG4(if(count!=-1)fprintf(stderr,"Amount written: %x \n", count))
	    DBG4(if(count==-1)fprintf(stderr,"write error %d \n", errno))
	    if(count==UERROR)
		{
		if (errno == EIO)
		    {
		    fprintf(stderr
		    ,"strm got a write I/O error which may indicate a bad volume \n");
		    DBG4(fprintf(stderr,"Amount written: %x\n",(filep-ofilep)))
		    if (filep-ofilep)
			{
			bufsize -= (filep-ofilep);
			}
		    }
		}
	    else
		{
		DBG4(fprintf(stderr,"Amount written: %x \n", count))
		bufsize -= count;
		}
	    if((Output = NextVol(1, Output))!=-1)
		{
		goto again;
		}
	    else
		strm_error("Could not write to the Output\n");
	    }
	if(!done)
	    {
	    DBG1(debugout("<Wp%ds%xComplete>\n",instance,count))
	    P_PREVSEM(instance); /* let the next one que it's write */
	    P_EMPTYSEM(instance); /* let the read routine fill us */
	    /* read should have filled us, and the next GETFULL won't block */
	    /* this may be as fast as we can queue the next write to a char */
	    /* special device; block devices hold off the next write by */
	    /* nature, might as well do it for both. */
	    }
	}
    DBG2(debugout("exiting write dev process %d\n",instance))
    (unsigned int)*(BDsize[NEXT(instance)])=0;
    P_FULLSEM(NEXT(instance)); 
    P_PREVSEM(instance); /* let the next one exit() */
    GETPREVSEM(PREV(instance)); 
    V_PREVSEM(PREV(instance)); 
    P_EMPTYSEM(instance); /* let the read routine exit() */
    DBG1(debugout("<Wp%ds%xDone>\n",instance,count))
    close(Output);
wdevexit:
    P_PREVSEM(instance); /* let the next one exit() */
    DBG2(debugout("exited write dev process %d\n",instance))
    exit(0);
}

/* IN option process routines */

static int readproc=0;
writeoutproc(pid)
int pid;
{
/*process is responsible for emptying the three stream buffers */
    unsigned int bufsize;
    int done=0;
    unsigned int count;

    DBG2(debugout("entering write out process\n"))
    P_PREVSEM(PREV(readproc)); /* let the first one que it's read */

    /* indicate all buffers are empty */
    do
	{
	P_EMPTYSEM(readproc);
	readproc = NEXT(readproc);
	}
    while(readproc);

    while (!done)
	{
	/* don't start emptying a buffer that is not full */
	GETFULLSEM(readproc); /* wait until the read routine fills us */
	V_FULLSEM(readproc); /* protect our emptying of the buffer */
	if((bufsize = (unsigned int)*(BDsize[readproc]))==0)
	    {
	    done++;
	    }
	if(!done && bufsize == Bufsize)
	    {
	    DBG1(debugout("<W%d-%xQueued>",readproc, bufsize))
	    if((count = write(Output, BDbuf[readproc], bufsize))!=bufsize) 
		{
		fprintf(stderr,"Amount written: %x \n", count);
		strm_error("Could not write the output\n");
		}
	    DBG1(debugout("<W%d-%xComplete>\n",readproc,count))
	    /* go on to next buffer/process */
	    P_EMPTYSEM(readproc);
	    readproc = NEXT(readproc);
	    }
	else 
	    {
	    DBG1(debugout("<W%d-%xDone>",readproc, count))
	    done++;
	    }
	}
    DBG1(debugout("s%xLast>\n",bufsize))
    DBG1(debugout("<W%d-%xQueued>",readproc, bufsize))
    if((count = write(Output, BDbuf[readproc], bufsize))!=bufsize) 
	{
	fprintf(stderr,"Amount written: %x \n", count);
	strm_error("Could not write the output\n");
	}
    close(Output);
    DBG2(debugout("exiting write out process\n"))
}

int readdone();	/* where we finish */

readevproc(instance)
    /* instance tells us which of processes 0, 1 or 2 we are */
    int instance;
{
    int done=0;
    unsigned int bufsize=0;

/* process is responsible for filling one of the three stream buffers */

    if((int)signal(SIGINT,readdone)==-1)
	{
	strm_error(" Can't catch SIGINT \n");
	}
    DBG2(debugout("entering read dev process %d\n",instance))
    GETEMPTYSEM(instance);
    while(!done)
	{
	if(GETPREVSEM(PREV(instance))==IERROR)
	    {
	    if(errno==36)
		{
		DBG2(debugout("Error exiting read dev process %d\n",instance))
		semctl(strm_semid[0][instance], 0, IPC_RMID);
		semctl(strm_semid[1][instance], 0, IPC_RMID);
		semctl(strm_semid[2][instance], 0, IPC_RMID);
		exit(0);
		}
	    }
	V_PREVSEM(PREV(instance)); 
	GETEMPTYSEM(instance);
	V_EMPTYSEM(instance);
	if(!(done=gatherdev(instance)))
	    {
	    if((bufsize= *BDsize[instance])==Bufsize)
		{
		P_FULLSEM(instance); 
		P_PREVSEM(instance); /* let the next one que it's read */
		}
	    else
		{
		P_FULLSEM(instance); 
		}
	    }
	else /* done */
	    {
	    P_FULLSEM(instance); 
	    P_PREVSEM(instance); /* let the next one que it's read */
	    V_EMPTYSEM(instance);
	    }
	}
rdevexit:
    DBG2(debugout("exiting read dev process %d\n",instance))
    exit(0);
}

static int retry=0;

gatherdev(instance)
int instance;
{
    unsigned int rv=0;
    int done=0;
    long ofilep, filep;	/* file pointer is the only way we know he's read */
    unsigned int in=0;

    DBG1(debugout("<gatherdev: Rp%ds%xQueued>\n",instance,Bufsize))
    while(!done)
	{
	ofilep = lseek (Input, (long)0, 1);	/* read original file pointer */
	if((rv=(unsigned int)read(Input, &(((unsigned char *)BDbuf[instance])
	 [in]), Bufsize - in)) != (Bufsize - in)) 
	    {
	    DBG1(debugout("<old filep: %lx>\n",ofilep))
	    if(rv == (unsigned int)-1)
		{
		filep = lseek (Input, (long) 0, 1);	/* read file pointer */
		DBG1(debugout("<filep: %lx>\n",filep))
		if (errno == EIO)
		    {
		    if(retry)
			{
			fprintf(stderr, "That didn't work\n"); 
			}
		    fprintf(stderr
		    ,"strm got a read I/O error which may indicate a bad volume \n");
		    DBG4(fprintf(stderr,"Amount read: %x\n",(filep-ofilep)))
		    if (filep-ofilep)
			{
			in += (filep-ofilep);
			}
		    rv = 0;
		    if((Input = NextVol(0, Input)) != -1)
			{
			retry++;
			DBG1(debugout("<Rp%d: new fd=%d>\n",instance,Input))
			}
		    }
		else
		    {
		    DBG1(debugout("<gatherdev: Read process %d got errno: %d>\n"
		     ,instance,errno))
		    done++;
		    }
		}
	    else if(rv == 0) 
		{
		filep = lseek (Input, (long) 0, 1);	/* read file pointer */
		DBG1(debugout("<filep: %lx>\n",filep))
		DBG1(debugout("<Rp%ds%xRetry>\n",instance,in+rv))
		if(retry)
		    {
		    fprintf(stderr, "That didn't work\n"); 
		    }
		if((Input = NextVol(0, Input)) != -1)
		    {
		    retry++;
		    DBG1(debugout("<Rp%d: new fd=%d>\n",instance,Input))
		    }
		}
	    else
		{
		DBG1(debugout("<gatherdev: Read process %d got %x, errno: %d>\n"
		 ,instance,rv,errno))
		done++;
		}
	    DBG1(debugout("<r%x/",rv))
	    in += rv;
	    rv = 0;
	    DBG1(debugout("t%x>",in))
	    }
	else
	    {
	    retry = 0;		/* successful read */
	    goto gatherexit;	/* exit but not done yet */
	    }
	}
gatherexit:
    *BDsize[instance] = (in+rv);
    DBG1(debugout("<gatherdev exit: Rp%ds%x%s>\n",instance
     , in+rv, done?"Done":"Complete"))
    return (done);
}


NextVol(mode, Volfd)
{
    register int Newfd;
    char str[22];
    FILE *devtty;
    struct stat statb;

    DBG4(debugout("NextVol errno: %d\n", errno))
    fstat(Volfd, &statb);
    if((statb.st_mode&S_IFMT) != S_IFCHR)
	{
	fprintf(stderr
	,"strm reached the end of %s, but its not a character special device.\n"
	 , devicename);
	fprintf(stderr
	 ,"Can't do multiple volumes - sorry.\n");
	return(-1);
	}
    fprintf(stderr
     ,"strm reached the end of the current Volume on %s\n"
     , devicename);
    fprintf(stderr
     ,"insert new Volume and hit <RETURN> to continue or <QUIT> to stop.\n");
    devtty = fopen("/dev/tty", "r");
    fgets(str, 20, devtty);
    if(*str&&(str[0]!='\n'))
	{
	fprintf(stderr,"using %s, can't switch devices in mid strm\n"
	 , devicename);
	}
    fclose(devtty);
#ifdef VOLCLOSE
    if(numprocs==1)	/* OK to close the device since only 1 process */
	{
	close(mode?Output:Input);
	if(!mode)
	    {
	    if((Input = open(devicename, O_RDONLY)) < 0) 
		{
		strm_error("cannot open <%s> \n", devicename);
		}
	    }
	else
	    {
	    if((Output = open(devicename, O_WRONLY)) < 0) 
		{
		strm_error("cannot open <%s> \n", devicename);
		}
	    }
	}
    else
#endif VOLCLOSE
	{
	lseek(Volfd,(long) 0,0);	/* start back at begining everyone */
	}
    return Volfd;
}


/*
 * Get a shared memory segment.
 * 
 */

unsigned char *
strmshmget (key, len, flags) /* returns zero if ENOMEM on shmat() */
key_t key;
unsigned len;
int flags;
{
    unsigned char *memaddr, *shmat();
    int shmid;

    if ((shmid = shmget (key, len, flags)) == IERROR) 
	{
	if (errno == ENOMEM)
	    {
	    return NULL;
	    }
	if (errno == EEXIST) 
	    {
	    fprintf (stderr, "Segmemt already exists\n");
	    return NULL;
	    }
	strm_error ("shmget (key=0x%lx, len=0x%x, flags=0x%x)\n"
	 , key, len, flags);
	}
    if ((memaddr = shmat (shmid, (unsigned char *) 0, 0)) == UCPERROR)
	{
	if (errno == ENOMEM)
	    memaddr = 0;
	else
	    strm_error ("");
	}

    strm_shmid[key-1] = shmid;
    return memaddr;
}

strmsem(semtype, id, opstructp)
int semtype;
int id;
struct sembuf **opstructp;
{
#ifdef SEMDEBUG
    char *tp="Bad type";
    char *sp="Bad operation";

    if (opstructp==(struct sembuf **)vsops)
	{
	sp = "V";
	}
    else if (opstructp==(struct sembuf **)psops)
	{
	sp = "P";
	}
    else if (opstructp==(struct sembuf **)gsops)
	{
	sp = "Conditional P";
	}

    switch(semtype)
	{
	case EMPTYSEM:
	    tp = "Empty Buffer";
	    break;
	case FULLSEM:
	    tp = "Full Buffer";
	    break;
	case PREVSEM:
	    tp = "Previous";
	    break;
	}

    DBG5(debugout("<process %d %s entry %s semid %d>\n", getpid(),
      sp, tp, strm_semid[semtype][id]))
#endif SEMDEBUG
    if(semval=semop(strm_semid[semtype][id],opstructp,NUMSEMS)==IERROR)
	{
	if(errno!=EIDRM&&errno!=EINVAL)
	    {
#ifdef SEMDEBUG
	    strm_error("<(errno:%d): process %d %s %s semid %d>\n"
	     , errno, getpid(), sp, tp, strm_semid[semtype][id]);
#else NOT SEMDEBUG
	    strm_error("<(errno:%d): process %d semid %d>\n"
	     , errno, getpid(), strm_semid[semtype][id]);
#endif SEMDEBUG
	    }
	}
#ifdef SEMDEBUG
    DBG5(debugout("<process %d %s exit %s semid %d = %d>\n", getpid(),
     sp, tp, strm_semid[semtype][id],semval))
#endif SEMDEBUG
    return(semval);
}

usage()
{
    fprintf(stderr
     ,"Usage: <i/o utility> | strm -o devicename \n%s\n",
    "       strm -i devicename | <i/o utility> ");
	exit(2);
}

isapipe(fd)
int fd;
{
    struct stat fdstat;

    if (fstat (fd,&fdstat) == -1)
	{
	fprintf (stderr
	 ,"cannot stat file descriptor %d (errno:%x)\n",fd,errno);
	exit(2);
	}
    return ((fdstat.st_mode&S_IFMT) & S_IFIFO);
}

#ifdef DEBUG
FILE *debugfd=stderr;
#endif DEBUG

debugopen()
{
#ifdef DEBUG

#ifdef DEBFILE
    char *dname="debug.out";

    if(access(dname, 02) == -1) 
	{
	strm_error("strm: no write access to <%s>\n", dname);
	}
    if((debugfd = fopen(dname, "w+")) < 0) 
	{
	strm_error("cannot open <%s> \n", dname);
	}
#endif DEBFILE
#endif DEBUG
}

debugout(s, p1, p2, p3, p4, p5, p6, p7, p8, p9)
char *s;
int p1,p2,p3,p4,p5, p6, p7, p8, p9;	/* ... if neccessary */
{
#ifdef DEBUG
    fprintf(debugfd, s, p1, p2, p3, p4, p5, p6, p7, p8, p9);
#endif DEBUG
}

debugclose()
{
#ifdef DEBUG
    close(debugfd);
#endif DEBUG
}

static strmdone(sig)
int sig;		/* signal # we caught */
{
    int i=numprocs;
    int waitret;

    DBG3(debugout("<strm done entry from %d SIGNAL>\n",sig))
    /* turn off possible re-entries from SIG_CLD */
    if((int)signal(SIGCLD,SIG_IGN)==-1)
	{
	strm_error(" Won't turn off SIGCLD \n");
	}
    DBG3(debugout("<strmdone: readproc=%d  out of %d numprocs>\n"
     , readproc, numprocs))

    while (i--)
	{
	if(childPID[i])
	    {
#ifdef READWAIT
	    if(i==PREV(readproc))
		{
		DBG3(debugout("<wait for process %d to finish read>\n"
		 , childPID[i]))
		/* don't kill a read in mid stream */
		GETFULLSEM(i); /* wait until read done */
		DBG3(debugout("<process %d finished read>\n"
		 , childPID[i]))
		}
#endif READWAIT
	    DBG3(debugout("<process %d being killed>\n", childPID[i]))
	    kill(childPID[i],SIGINT);
	    DBG3(debugout("<process %d was killed>\n", childPID[i]))
	    }
	} /* end child kill loop */
#ifdef REMOVE					/* M000 */
    if (sig == SIGPIPE)
	{
	if (PREV(readproc) != 0)		/* M000 prevent sem hang */
		P_PREVSEM(PREV(readproc)); /* let the read procs die */
	}
#endif REMOVE					/* M000 */
#ifdef READWAIT
    i=numprocs;
    while (i--)
	{
	if(childPID[i])
	    {
	    DBG3(debugout("<entering wait for process %d to die>\n"
	     , childPID[i]))
	    if((waitret=wait(&childSTAT[i])) != childPID[i])
		{
		DBG3(debugout("<strmdone: wait returned %d>\n"
		 , waitret))
		if(waitret == IERROR)
		    {
		    switch (errno)
			{
			case EINTR:
			    {
			    /* got the signal and died */
			    DBG3(debugout("<strmdone: EINTR>\n"))
			    goto nextchild;
			    /* no getum here, (quick way to break out)*/
			    }
			    break;

			case ECHILD:
			    {
			    /* no more children, might as well die */
			    DBG3(debugout("<strmdone: ECHILD>\n"))
			    goto doneexit;
			    /* no getum here, (quick way to break out)*/
			    }
			    break;

			default:
			    DBG3(debugout("<strmdone: wait returned errno %d>\n"
			     , errno))
			    goto nextchild;
			    break;
			}
		    }
		} /* end wait on child i */
	    DBG3(debugout("<process %d died on %d wait>\n", waitret
	     , childPID[i]))
	    }
nextchild:
	continue;
	} /* end child wait loop */
#endif READWAIT
doneexit:
    DBG3(debugout("<process %d doneexit>\n", getpid()))
    strm_exit(0);
}

strm_error(s, p1, p2, p3, p4, p5)
char *s;
int p1,p2,p3,p4,p5;	/* ... if neccessary */
{
    fprintf(stderr, s, p1, p2, p3, p4, p5);
    perror("strm");
    strm_exit(2);
}

readdone(sig)
int sig;
{
    DBG3(debugout("<readdone: pid: %d SIGNAL %d>\n", getpid(),sig))
    if((int)signal(SIGINT,SIG_IGN)==-1)
	{
	strm_error(" Can't stop SIGINT \n");
	}
    debugclose();
    close(Input);
    exit(0);
}

strm_exit(exit_value)
int exit_value;
{
    while(numprocs--)
	{
	shmdt((unsigned char *)(BDbuf[numprocs]-sizeof(unsigned int)));
	shmctl(strm_shmid[numprocs], IPC_RMID, 0);
	semctl(strm_semid[0][numprocs], 0, IPC_RMID);
	semctl(strm_semid[1][numprocs], 0, IPC_RMID);
	semctl(strm_semid[2][numprocs], 0, IPC_RMID);
	}
    debugclose();
    close(Input);
    close(Output);
    exit(exit_value);
}

static strmrdone()
{
    int i=numprocs;
    int waitret;

    while (i--)
	{
	if(childPID[i])
	    {
	    kill(childPID[i],SIGKILL);
	    }
	} /* end child kill loop */
    i=numprocs;
    while (i--)
	{
	if(childPID[readproc])
	    {
	    if((waitret=wait(&childSTAT[readproc])) != childPID[readproc])
		{
		if(waitret == IERROR)
		    {
		    switch (errno)
			{
			case EINTR:
			    {
			    /* got the signal and died */
			    goto nextchild;
			    /* no getum here, (quick way to break out)*/
			    }
			    break;

			case ECHILD:
			    {
			    /* no more children, might as well die */
			    goto doneexit;
			    /* no getum here, (quick way to break out)*/
			    }
			    break;

			default:
			    goto nextchild;
			    break;
			}
		    }
		} /* end wait on child readproc */
	    }
nextchild:
	readproc = NEXT(readproc);
	continue;
	} /* end child wait loop */
doneexit:
    strm_exit(0);
}
