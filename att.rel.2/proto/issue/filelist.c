/*	@(#)filelist.c	1.1	85/06/02		*/
/***********************************************************************
*
* filelist utility
*
* This utility takes the files doc, and rejects the field not
* required, and rejects the entries for files which do not exist
* ( a file does not exist if size field is silly ie } or ? )
* this was an awk, but was too slow!
*
*
***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

char name[100];					/* file name field, up to tab */
char sz[100];					/* field after file name      */
char ps[100];					/* product set of file        */

long mkint( s )
char s[];
/* to return the size field of the file with name name,
*/
{
	int i;
	long n;

	for (i=0; s[i]==' ' || s[i]=='}' ; i++) ;

	for (n=0; s[i]>='0' && s[i]<='9' ; i++) 
		n = n*10 + s[i] -'0';

	return(n);
} /* end of mkint */


void find_next_line()
{
	char c;

	while ( (c=getchar()) != '\n' && c != EOF )
		;
} /* end of find_next_line */




main ( argc, argv )
int argc;
char **argv;
{
	char c;
	int i;
	long size = 0;

	name[0] = '\0';					/* initialise name to be empty */

	while ( (c=getchar()) != EOF )
	{
		if ( !(c == '/' || c == '"') )
		{	/* not a line with files on, throw this line away */
			if ( c != '\n' ) find_next_line();
		}
		else
		{
			if (c=='"')				
				for ( ;(c=getchar())!='\t' && c != EOF; );
			else
			{							/* read in name */
				name[0] = c;
				for (i=1;(c=getchar())!='\t' && c != EOF; i++) name[i] = c;
				name[i] = 0;
			}

			for (i=0;(c=getchar())!='\t' && c != EOF; i++) sz[i] = c;
			sz[i] = 0;

			for ( ;(c=getchar())!='\t' && c != EOF; ) ; /* skip svid */

			while ((c=getchar())=='{' || c==' ')		/* skip {    */
				;

			for (i=0;c != '\t' && c != EOF; i++)
			{
				ps[i] = c;
				c=getchar();
			}
			ps[i] = 0;

			find_next_line();

			if (! (ps[0] == '"' && ps[1] == 0))
			{
				if ((sz[0] == '?' && sz[1] ==0) || strcmp(sz,"} ?") == 0)
				{
					fprintf(stderr,"%s not found\n",name);
					size = -1;
				}
				else
				{
					if ((sz[0]=='"' || sz[0]=='}') && sz[1]==0 && size==-1)
						fprintf(stderr,"%s not found\n",name);
					else
					{
						if (! (sz[1] == 0 && (sz[0]=='}' || sz[0]=='"')) )
							size=mkint(sz);
						printf("%s\t%s\t%ld\n",name,ps,size);
					}/* end if */
				}/* end if */
			}/* end if file in new ps */
		}
	} /* end of c = EOF */
} /* end of main */
