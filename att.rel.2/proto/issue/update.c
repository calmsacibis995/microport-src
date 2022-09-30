/*		@(#)update.c	1.2 85/07/02		*/
/***********************************************************************
*
* update utility
*
* this utility is used to update the files document, by
* ignoring the current filesize field, using status to
* get the correct size and writing the original line with
* the new size.
*
*
***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define tab =`\t`;

char name[100];					/* file name field, up to tab */
char next[100];					/* field after file name      */

long getsz()
/* to return the size of the file with name name,
*  returns -1 if file doesn't exist
*/
{
	struct stat status;

	if (stat(name,&status) == -1)
		return(-1);
	else return (status.st_size);

} /* end of getsz */


void copy_line()
{
	char c;

	while ( (c=getchar()) != '\n' && c != EOF )
		putchar(c);
	putchar('\n');
} /* end of copy_line */




main ( argc, argv )
int argc;
char **argv;
{
	char c;
	int i;
	long size;

	while ( (c=getchar()) != EOF )
	{
		if ( c != '/' )
		{
			putchar(c);
			if ( c != '\n' ) copy_line();
		}
		else
		{
			name[0] = c;
			for (i=1;(c=getchar())!='\t' && c != EOF; i++)
				name[i] = c;
			name[i] = 0;
			for (i=0;(c=getchar())!='\t' && c != EOF; i++)
				next[i] = c;
			next[i] = 0;

			if ( next[0]=='}' && i == 1 )
				printf("%s\t}\t",name);
			else
			{
				size = getsz();
				if ( size != -1 )
				{
					if ( next[0] == '}' )
						printf("%s\t} %ld\t",name,size);
					else
						printf("%s\t%ld\t",name,size);
				}
				else if ( next[0] == '}' )
						printf("%s\t} ?\t",name);
					 else printf("%s\t?\t",name);
			}
			copy_line();
		}
	} /* end of c = EOF */
} /* end of main */
