/*
 * The BASECHECK program is designed to completely verify a release.
 * The file that it uses as input is the distribution
 * dictionary for the release. According to the options it is given, it:
 *
 *     -m              - Modify the modes and owners
 *     -pPASSWD        - use this for the target machine password file.
 *     -gGROUP         - use this for the target machine group file.
 *     -sMAGIC1,MAGIC2 - sum & check magic nos.
 *     -oOUTFILE       - echo the basefile to stdout, optionally to OUTFILE
 *     -d              - description matching.
 *
 * The magic numbers are found in /usr/include/filehdr.h
 * WARNING: the numbers in /usr/include/filehdr.h are octal, I expect
 * to get decimal numbers. (currently, for iAPX286 the nos are 330,338).
 * The acceptable description list is found at the end of the program
 * and can be changed by modifying the table nocheck.
 * The program assumes that the BASE file has been sorted into order by the
 * file name. Unexpected diagnostics may result from the file being out of
 * order.
 * NOTE: This program assumes COFF.
 * PROBLEM: Checking the file type is very minimal.
 */
#define MAXPATH 128 + 1
#define MAXSET 16 + 1
#define MAXPERM 11 + 1
#define MAXUSER 14 + 1
#define MAXGROUP 14 + 1
#define MAXDESC 34 + 1
#include <stdio.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#undef NULL
#include <sys/param.h>
#include <filehdr.h>

typedef struct pair_t {
   ushort uid;
   char *name;
} pair_t;
typedef struct pass_t {
   short int length;
   pair_t *array;
} pass_t;

extern int errno;
char *strtok(), *malloc(), *strcpy(), *strcat(), *strchr();
long atol();
pass_t *initp(), *passlist, *grouplist;
char *reverse(); /*forward*/
long sum();
char sflg=0;
char oflg=0;
char mflg=0;
char dflg=0;
char *PASSWD = "/etc/passwd";
char *GROUP = "/etc/group";
char *root = "";
int linecount=0;
jmp_buf doneline;
int errcount=0;
unsigned short magic1 = -1,magic2 = -1;

main(argc,argv)
int argc;
char **argv;
{
   int ch;
   extern int optind;
   extern char *optarg;

   setuid(0);
   while((ch=getopt(argc,argv,"dg:mo:p:s:")) != EOF) {
      switch(ch) {
	 case 'd':
	    dflg=1;
	 break;
	 case 'm':
	    mflg=1;
	 break;
	 case 's':
	    sflg=1;
	    magic1=atoi(optarg);
	    optarg = strchr(optarg,',');
	    if (optarg) magic2=atoi(optarg+1);
	 break;
	 case 'p':
	    PASSWD=optarg;
	 break;
	 case 'g':
	    GROUP=optarg;
	 break;
	 case 'o':
	    oflg=1;
	    if (optarg)
	       if (freopen(optarg,"w",stdout)==NULL) {
		  fprintf(stderr,"Cannot reopen stdout as %s\n",optarg);
		  exit(1);
	       }
	 break;
	 case '?':
	    exit(1);
      }
   }
   if (mflg && geteuid()) fprintf(stderr,"Hey whats this? I am not setuid root!\n");
   if (optind < argc-1) {
      fprintf(stderr,"Illegal extra arguments in command line\n");
      exit(1);
   }
   root=argv[optind];
   if (isatty(0)) {
      fprintf(stderr,
      "USAGE: <BASE basecheck [-m] [-d] [-oBASE.new] [-pPASSWD] [-gGROUP]\
 [-sMAGIC1,MAGIC2]\n",*argv);
      exit(1);
   }
   passlist = initp(PASSWD,256);
   grouplist = initp(GROUP,256);
   for(;;) { /*main loop*/
      char source[MAXPATH],dest[MAXPATH];
      char a[MAXSET],b[MAXSET],c[MAXSET],d[MAXSET];
      char perm[MAXPERM];
      char user[MAXUSER];
      char group[MAXGROUP];
      char desc[MAXDESC];
      if (!getfield(a,sizeof a)) done(0);
      ++linecount;
      if (!getfield(b,sizeof b)) done(1);
      if (!getfield(c,sizeof c)) done(1);
      if (!getfield(d,sizeof d)) done(1);
      if (!getfield(perm,sizeof perm)) done(1);
      if (!getfield(user,sizeof user)) done(1);
      if (!getfield(group,sizeof group)) done(1);
      if (!getfield(desc,sizeof desc)) done(1);
      if (!getfield(dest,sizeof dest)) done(1);
      if (!getfield(source,sizeof source)) done(1);
      if (*a != '#')
	 doone(a,perm,user,group,desc,dest,source);
      if (oflg)
	 printf("%s %s %s %s %s %s %s %s %s %s\n",
		a,b,c,d,perm,user,group,desc,dest,source);
      if (!newline()) done(0);
   }
}


getfield(s,size)
char *s;
int size;
{
   register int c;
   register char *p, *end;

   if (!getblanks()) return 0;
   p=s;
   end=s+size;
   while (p<end) {
      c=getchar();
      if (c==' ' || c=='\t' || c=='\n' || c=='\f' || c==EOF) {
	 *p++='\0';
	 ungetc(c,stdin);
	 return 1;
      }
      *p++ = c;
   }
   fprintf(stderr,"String overflow\n");
   return 0;
}


newline()
{
   int c;
   while( (c=getchar())!='\n' && c!='\f' && c!=EOF);
   return c!=EOF;
}


getblanks()
{
   int c;
   while( (c=getchar()) ==' ' || c=='\t' );
   if (c==EOF) return 0;
   ungetc(c,stdin);
   return 1;
}


doone(a,perm,user,group,desc,destfile,sourcefile)
char *a,*perm,*user,*group,*desc,*destfile,*sourcefile;
/*
 * Handle one file. There are 3 variables of concern. These are:
 *   (1) mflg (ON,OFF)
 *   (2) file type is f,d,c,b,p
 *   (3) source-file is "-" or indicates a template
 *
 * The following outlines the treatment of the program:
 *   A. mflg OFF
 *      bcdp. Create full dest pathname.
 *         Makenode or makedir will check the stats.
 *      f. Create full pathname.
 *         1. Source file is -: create full pathname for
 *            verification of the file type & modes check.
 *         2. FILENAMES differ - create full pathnames for both.
 *            cmp to find differences.
 *  B. mflg ON
 *     bcp. Create full pathname.
 *        If it is there, remove it.
 *        makenode.
 *     d. Check it is a directory.
 *     f. Create both pathnames.
 *        (1) Non"-" source file:
 *            If file is there, remove and replace, copy, set modes.
 *        (2) "-" source file
 *               Create full path name so that a check can be done.
 */
{
   char source[MAXPATH], dest[MAXPATH];
   struct stat deststat, sourcestat;
   int i;
   ushort u,g;
   char command[MAXPATH*2+5];

   catname(root,destfile,dest,sizeof dest);
   /* If we return from an error, simply return to the caller */
   if (!setjmp(doneline)) {
      if (*a=='?')
	 error("Query outstanding for file %s",dest);
      if (mflg) { /* modify modes and owners */
	 switch(*perm) {
	    case 'p':
	       if (!stat(dest,&deststat)) {
		  sprintf(command,"rm %s\n",dest);
		  if (system(command))
		     error("Cannot remove %s",dest);
	       }
	       if (mknod(dest,convtype(*perm)|convperm(perm),0))
		  error("Cannot mknod %s",dest);
	    break;
	    case 'b':
	    case 'c':
	       /* If there is a special file there already, blast */
	       if (!stat(dest,&deststat)) {
		  sprintf(command,"rm %s\n",dest);
		  if (system(command))
		     error("Cannot remove %s",dest);
	       }
	       if (mknod(dest,convtype(*perm)|convperm(perm),
			 convdev(sourcefile,dest)))
		  error("Cannot mknod %s",dest);
	    break;
	    case 'd':
	       if (!stat(dest,&deststat))
		  if ((deststat.st_mode & S_IFMT) != S_IFDIR)
		     error("%s NOT A DIRECTORY",dest);
		  else;
	       else if (sprintf(command,"mkdir %s\n",dest),system(command))
		  error("Cannot mkdir %s",dest);
	    break;
	    case '-':
	    case 'f':
	       if (!strncmp("LINK_",sourcefile,5)) {
		  catname(root,sourcefile+5,source,sizeof source);
		  if (stat(source,&sourcestat)) {
		     /* source missing, check dest */
		     if (stat(dest,&deststat))
			error("Both source %s and dest %s missing",
			       sourcefile,dest);
		     else {
			warning("Link reversed - %s missing, %s present",
				 sourcefile,dest);
			if (link(dest,source))
			   error("Cannot reverse link %s to %s",source,dest);
		     }
		  }
		  if (!stat(dest,&deststat)) { /* new file there already */
		     if (sourcestat.st_dev != deststat.st_dev ||
			 sourcestat.st_ino != deststat.st_ino ||
			 sourcestat.st_nlink<2)
		     { /* different file is there, so check it out */
			if (deststat.st_size == 0) ; /*don't compare*/
			else if (sourcestat.st_size != deststat.st_size)
			   error("File %s not same size as %s",dest,source);
			else {
			   sprintf(command,"cmp -s %s %s\n",source,dest);
			   if (system(command))
			      error("File %s not identical to %s",
				    dest,source);
			}
			warning("File %s removed before linking to %s",
				dest,source);
			sprintf(command,"rm %s\n",dest);
			if (system(command))
			   error("Cannot remove %s",dest);
			if (link(source,dest))
			   error("Cannot link %s to %s",dest,source);
		     }
		     else ; /* already linked properly */
		  }
		  else if (link(source,dest))
		     error("Cannot link %s to %s",dest,source);
		  else ; /* link succeeded */
	       }
	       if (stat(dest,&deststat)) error("MISSING %s",dest);
	    break;
	    default:
	       error("%s has an illegal file type",dest);
	 }
	 if (chown(dest,convuser(user),convgrp(group)))
	    error("Cannot chown %s to user %s and group %s",
	       dest,user,group);
	 if (chmod(dest,convperm(perm)))
	    error("Cannot chmod %s to %s",dest,perm);
      }  /* end of mflg==1 */

      else {
	 /*
	  * The destfile name must be supplied and exist,
	  * although the source filename is a device description for
	  * special files. Get the stat of the destination.
	  */
	 if (stat(dest,&deststat))
	    if (errno==2)  error("MISSING %s", dest);
	    else error("Cannot stat %s, errno=%d",dest,errno);

	 /* Check the type of file found */
	 if ((deststat.st_mode&S_IFMT) != convtype(*perm))
	    error("%s is not of the proper type",dest,*perm);

	 /*
	  * If it is a device  file, check the device numbers.
	  * If it is a regular file, check the template.
	  */
	 switch(*perm) {
	    case 'c':
	    case 'b':
	    case 'p':
	       devchk(deststat.st_rdev,sourcefile,dest);
	    break;
	    case '-':
	    case 'f':
		if (!strncmp(sourcefile,"LINK_",5)) {
		   catname(root,sourcefile+5,source,sizeof source);
		   if (stat(source,&sourcestat))
		      error("Cannot stat %s LINK source file %s",dest,source);
		   if (sourcestat.st_dev != deststat.st_dev ||
		       sourcestat.st_ino != deststat.st_ino ||
		       sourcestat.st_nlink<2
		      )
		      error("%s NOT LINKED to %s",dest,source);
		}
	    break;
	    /*default unnecessary because convtype checks above*/
	 }
	 if ((i=convperm(perm)) != (deststat.st_mode & ~S_IFMT))
	    error("mode of %s was %o not %o",dest,deststat.st_mode & ~S_IFMT, i);
	 if ((g=convgrp(group)) != deststat.st_gid)
	    error("group of %s was %s not %s",dest,
		  reverse(deststat.st_gid,grouplist),
		  reverse(g,grouplist));
	 if ((u=convuser(user)) != deststat.st_uid)
	    error("owner of %s was %s not %s",dest,
		   reverse(deststat.st_uid, passlist),
		   reverse(u,passlist));
      }

      if (sflg) switch(*perm) {
	 case 'd':
	    if (strcmp(desc,"dir"))
	       error("Directory %s does not have a dir description",dest);
	 break;
	 case 'b': case 'c':
	    if (strcmp(desc,"dev"))
	       error("Device %s does not have a dev description",dest);
	 break;
	 case 'p':
	    if (strcmp(desc,"fifo"))
	       error("Device %s does not have a fifo description",dest);
	 break;
	 case '-':
	 case 'f':
	    if (sflg) {
	       unsigned short newsum;
	       long newblocks,newbytes;
	       char *oldsum, *oldblocks, *oldbytes;

	       oldsum=0; oldblocks=0; oldbytes=0;
	       switch(checksum(dest,&newsum,&newblocks,&newbytes,desc)) {
		  case 3:
		     error("Checksum read error %d, %s",errno,dest);
		  break;
		  case 2:
		     error("Checksum close error %d, %s",errno,dest);
		  break;
		  case 1:
		     error("Checksum open error %d, %s",errno,dest);
		  break;
		  case 0:
		     if (oldsum = strchr(desc,',')) {
                        long ioldbytes,ioldblocks;
                        if (newsum != atoi(oldsum+1))
                           error("CHECKSUM ERROR %ud!=%s, %s",
                                 newsum,oldsum+1,dest);
                        if (!(oldblocks=strchr(oldsum+1,',')))
                           error("Number of blocks missing");
                        ioldblocks=atol(oldblocks+1);
                        if (ioldblocks != newblocks)
                           error("BLOCKSIZE ERROR %ld!=%ld, %s",
                                 newblocks,ioldblocks,dest);
                        if (!(oldbytes=strchr(oldblocks+1,','))) 
                           error("Number of bytes missing");
                        ioldbytes=atol(oldbytes+1);
                        if (ioldbytes != newbytes) 
                           error("BYTESIZE ERROR %ld!=%ld, %s",
                                 newbytes,ioldbytes,dest);
		     }
		     if (!oldsum) oldsum=desc+strlen(desc);
		     if (oflg)
			sprintf(oldsum,",%u,%ld,%ld",(unsigned) newsum,newblocks,newbytes);
		  break;
		  default: error("Internal error");
	       }
	    }
	 break;
	 default: error("Internal error2");
      }
	 /* If we got here, everything succeeded */
   }  /* end of setjmp */
}


done(n)
{
   fprintf(stderr,"Exit %d\n",n+errcount);
   exit(!!(n|errcount));
}


/*VARARGS1*/
error(s1,s2,s3,s4,s5,s6)
char *s1;
{
   fprintf(stderr,"%d:",linecount);
   fprintf(stderr,s1,s2,s3,s4,s5,s6);
   putc('\n',stderr);
   ++errcount;
   longjmp(doneline,1);
}


/*VARARGS1*/
warning(s1,s2,s3,s4,s5,s6)
char *s1;
{
   fprintf(stderr,"%d:",linecount);
   fprintf(stderr,s1,s2,s3,s4,s5,s6);
   putc('\n',stderr);
}


#undef major
#undef minor
convdev(sourcefile,dest)
char *sourcefile,*dest;
{
   register char *s=sourcefile, *next;
   int major,minor;

   if ((next=strchr(s,',')) == (char *)NULL)
      error("Major,Minor device numbers wrong/missing, %s",dest);
   *next='\0';
   if (!verify(s))
      error("Illegal major device number %s for %s",sourcefile,dest);
   major=atoi(s);
   *next++ = ',';
   if (!verify(next))
      error("Illegal minor device number %s for %s",sourcefile,dest);
   minor=atoi(next);
   return (major<<8) | minor;
}


verify(s)
char *s;
{
   while(*s && *s>='0' && *s<='9') s++;
   return !*s;
}


convperm(perm)
char *perm;
{
   register int result=0;
   register char *m=perm;

   ++m;
   if (*m=='r') result |= S_IREAD;
   else if (*m=='-') ;
   else error("Illegal permission %s",perm);

   ++m;
   if (*m=='w') result |= S_IWRITE;
   else if (*m=='-') ;
   else error("Illegal permission %s",perm);

   ++m;
   if (*m=='x') result |= S_IEXEC;
   else if (*m=='s') result |= (S_IEXEC|S_ISUID) ;
   else if (*m=='-') ;
   else error("Illegal permission %s",perm);

   ++m;
   if (*m=='r') result |= (S_IREAD >> 3);
   else if (*m=='-') ;
   else error("Illegal permission %s",perm);

   ++m;
   if (*m=='w') result |= (S_IWRITE >> 3);
   else if (*m=='-') ;
   else error("Illegal permission %s",perm);

   ++m;
   if (*m=='x') result |= (S_IEXEC >> 3);
   else if (*m=='g' || *m=='s') result |= ((S_IEXEC >> 3) | S_ISGID);
   else if (*m=='-') ;
   else error("Illegal permission %s",perm);

   ++m;
   if (*m=='r') result |= (S_IREAD >> 6);
   else if (*m=='-') ;
   else error("Illegal permission %s",perm);

   ++m;
   if (*m=='w') result |= (S_IWRITE >> 6);
   else if (*m=='-') ;
   else error("Illegal permission %s",perm);

   ++m;
   if (*m=='x') result |= (S_IEXEC >> 6);
   else if (*m=='-') ;
   else error("Illegal permission %s",perm);

   ++m;
   if (*m=='t') result |= S_ISVTX;
   else if (*m) error("Illegal permission %s",perm);
   return result;
}


convgrp(group)
char *group;
{
   int g;
   if (!puid(group,&g,grouplist))
      error("Cannot find group %s in group file",group);
   return g;
}


convuser(user)
char *user;
{
   int u;
   if (!puid(user,&u,passlist))
      error("Cannot find user %s in passwd file", user);
   return u;
}


catname(d,f,s,slen)
char *d,*f,*s;
int slen;
{
   /* Do both names fit with a trailing null added? */
   if ( strlen(d)+strlen(f)+1 > slen)
      error("dir & file names too long %s/%s",d,f);
   strcpy(s,d);
   strcat(s,f);
}


devchk(statdev,sourcefile,dest)
int statdev;
char *sourcefile, *dest;
{
   register int dev;
   if ( (dev=convdev(sourcefile,dest)) != statdev)
       error("Major,Minor device number %d,%d of %s doesn't match %d,%d",
	  (statdev>>8) & 0xff, statdev & 0xff,dest,
	      (dev>>8) & 0xff, dev & 0xff);
}


convtype(t)
char t;
{
   switch(t) {
      case 'b':
	 return S_IFBLK;
      case 'c':
	 return S_IFCHR;
      case 'd':
	 return S_IFDIR;
      case 'p':
	 return S_IFIFO;
      case 'f':
      case '-':
	 return S_IFREG;
      default:
	 error("Illegal unknown file type %c",t);
   }
   return 0;
}


pass_t *initp(filename,limit)
char *filename;                /*password file name*/
int limit;                     /*array size, do not exceed*/
{
   char *newname;
   int newuid, newpos;
   FILE *pwf;
   pair_t *pair;
   pass_t *pass;

   if ((pwf=fopen(filename, "r")) == NULL) {
      fprintf(stderr,"Cannot open password file\n");
      return (pass_t *) 0;
   }
   pair = (struct pair_t *) malloc((unsigned) sizeof(pair_t)*limit);
   pass = (struct pass_t *) malloc((unsigned) sizeof(pass_t));
   pass->array= pair;
   pass->length = 0;

   while( nextp(pwf,&newname,&newuid) ) {
      if (pass->length >= limit) {
	 fprintf(stderr,"Too many passwords\n");
	 return (pass_t *) 0;
      }
      if (plocate(newname,&newpos,pass))
	 fprintf(stderr,"Duplicate name %s found\n",newname);
      else {
	 /* returned position just ahead of where it goes */
	 int i;
	 for (i= pass->length - 1; i>=newpos; --i) pair[i+1] = pair[i];
	 pair[newpos].name=strcpy(malloc((unsigned)strlen(newname)+1),newname);
	 pair[newpos].uid=newuid;
#        ifdef DEBUG
	    fprintf(stderr,"pair[newpos].name=%08x,%s, uid=%d, newpos=%d\n",
	       pair[newpos].name,pair[newpos].name,pair[newpos].uid,newpos);
#        endif
      }
      ++(pass->length);
   }
   if (fclose(pwf)) {
      fprintf(stderr,"Can't close password file\n");
      return (pass_t *) 0;
   }
   return pass;
}


static char *
pwskip (param)
char *param;
{
	register char *p;
	p=param;
	while (*p && *p != ':' && *p != '\n')
		++p;
	if (*p == '\n')
		*p = '\0';
	else if (*p)
		*p++ = '\0';
	return p;
}


nextp(pwf,name,uid)
FILE *pwf;
char **name;
int *uid;
{
	register char *p;
	static char line[BUFSIZ+1];

	if ((p = fgets (line, BUFSIZ, pwf))==NULL) return 0;
	*name = p;
	p = pwskip (p);
	p = pwskip (p);
	*uid = atoi (p);
	return 1;
}


plocate(sought,pos,pass)
char *sought;
int *pos;
pass_t *pass;
{
   register short int result, middle;
   register short int low=0, high=pass->length-1;
   register pair_t *pair=pass->array;
#  ifdef DEBUG
      fprintf(stderr,"pass=%08x, pass->array=%08x, pass->length=%d\n",
		      pass,pass->array,pass->length);
      fprintf(stderr,"pair=%08x, pair[0].name=%s, pair[0].uid=%d\n",
		      pair,pair[0].name,pair[0].uid);
#  endif
   result=0;
   while(low <= high) {
      middle = (low+high) / 2;
#     ifdef DEBUG
	 fprintf(stderr,"compare low=%d, middle=%d, result=%d, high=%d, %s %s\n",
	    low,middle,result,high,sought,pair[middle].name);
#     endif
      result = strcmp(sought,pair[middle].name);
      if (result<0) {
	 result = middle;
	 high = middle - 1 ;
      }
      else if (result>0) {
	 result = middle + 1;
	 low = middle + 1;
      }
      else {
	 *pos = middle;
	 return 1;
      }
   }
   *pos=result;
   return 0;
}


puid(sought,result,pass)
char *sought;
int *result;
pass_t *pass;
{
   int pos;
   if (!plocate(sought,&pos,pass)) return 0;
   *result = pass->array[pos].uid;
   return 1;
}


char *reverse(sought,pass)
ushort sought;
pass_t *pass;
{
   int pos;
   for (pos=0; pos<pass->length; pos++)
      if(sought==pass->array[pos].uid)
	 return pass->array[pos].name;
   return "UNKNOWN";
}


checksum(filename,sum,blocks,size,desc)
char *filename;
unsigned short *sum;
long *blocks;
long *size;
char *desc;
/*
 * Sum bytes in file mod 2^16
 */
{
   register unsigned result;
   register unsigned char *pbuf;
   int f,n;
   long nbytes;
   unsigned char buf[BUFSIZ], *endbuf;

   if ( (f = open(filename, O_RDONLY)) == -1) return 1;
   result = 0;
   nbytes = 0;
   n=read(f,buf,sizeof buf);
   identify(buf,filename,n,desc);
   while(n > 0) {
      nbytes += n;
      endbuf = buf + n;
      for(pbuf=buf; pbuf<endbuf; pbuf++) {
	 if (result & 01) result = (result >> 1) + 0x8000;
	 else result >>= 1;
	 result += (unsigned) *pbuf;
	 result &= 0xFFFFL;
      }
      n=read(f,buf,sizeof buf);
   }
   if (close(f)) return 2;
   if (n<0) return 3;
   *sum = result;
   *blocks = (nbytes+512-1) / 512;
   *size = nbytes;
   return 0;
}


identify(buf,filename,nbytes,desc)
char *buf,*filename, *desc;
int nbytes;
{
   struct filehdr *f = (struct filehdr *) buf;
   char *comma = strchr(desc,',');
   if (comma) *comma='\0';

   /*
    * Check first for zero length files to control the effects of :mktouch
    */
   if (!strcmp(desc,"null"))
      if (nbytes!=0)
	 warning("Null file %s does not have zero bytes",filename);
      else;
   else if (nbytes==0)
      warning("Zero length file %s of type %s",filename,desc);

   /*
    * If it has length, make sure that only binaries have magic numbers
    */
   else if (!strcmp(desc,"bin") || !strcmp(desc,"obj"))
      if (nbytes < sizeof (struct filehdr))
	 warning("Truncated binary %s",filename);
      else if (f->f_magic != magic1 && f->f_magic != magic2)
	 warning("Illegal magic number for %s",filename);
      else if (strcmp(desc,"obj") &&
               (f->f_flags & (F_LNNO | F_LSYMS)) != (F_LNNO | F_LSYMS) )
	 warning("File %s not stripped",filename);
      else;
   else if (nbytes >= sizeof (struct filehdr) &&
      f->f_magic==magic1 || f->f_magic==magic2 )
      warning("File %s has a magic number but is not a binary",filename);

   /*
    * Make sure archives are what they say they are
    */
   else if (!strcmp(desc,"arch"))
      if (nbytes < 7 || strncmp(buf,"!<arch>",7))
	 warning("Archive file %s does not have archive header",filename);
      else;
   else if (nbytes > 7 && !strncmp(buf,"!<arch>",7))
	 warning("Archive file %s is not called 'arch'",filename);

   /*
    * Check for suffix regularity.
    */
   else if (dflg) {
      static struct {
	 char *descabbr, *suffix;
      } *parray, array[] = {
	 "asrc",".s",
	 "csrc",".c",
	 "incl",".h",
	 "isrc",".h",
	 "lsrc",".l",
	 "ysrc",".y",
	 "ssrc",".sh",
	 "atxt",0,
	 "cman",0,
	 "conf",0,
	 "ctxt",0,
	 "data",0,
	 "dsrc",0,
	 "fsrc",0,
	 "llib",0,
	 "lsrc",0,
	 "make",0,
	 "man",0,
	 "ntxt",0,
	 "pman",0,
	 "read",0,
	 "tic",0,
	 "obj",0,
	 0,0 };
	 /* dev, dir and fifo are handled in doone*/
      int founddesc=0;
      for (parray=array; parray->descabbr; parray++) {
	 if (!strcmp(parray->descabbr,desc)) {
	    founddesc=1;
	    if (parray->suffix) {
	       int sufflen=strlen(parray->suffix);
	       int namelen=strlen(filename);
	       if ( strcmp(parray->suffix,filename+(namelen-sufflen)) )
		  warning("File %s of type %s does not have suffix %s",
			  filename, desc, parray->suffix);
	    }
	 }
      }
      if (!founddesc) warning("File %s has illegal description %s",
			      filename,desc);
   }
   if (comma) *comma=',';
}
