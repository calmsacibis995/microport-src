/* uportid = "@(#)misc.h	Microport Rev Id  1.3.3 6/18/86" */

#define WAIT {int i,j; for (i=100;i;i--) for (j=0x7fff;j;j--);}
#define BUG(string) if (lwdb > 0) {long i,j; printf ("%s\n",string);for(i=0x1ffff; i ;i--  );}
#define BUGG(string)  /*{int i,j; printf ("%s\n",string);for(i=0xffff; i ;i--  );}*/
#define BOMB(string) {printf ("\n%s\n",string);return (1);}
typedef unsigned char byte;
 extern int lwdb;
