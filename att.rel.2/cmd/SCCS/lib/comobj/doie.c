/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
# include	"../../hdr/defines.h"


doie(pkt,ilist,elist,glist)
struct packet *pkt;
char *ilist, *elist, *glist;
{
	if (ilist) {
		if (pkt->p_verbose & DOLIST)
			fprintf(pkt->p_stdout,"Included:\n");
		dolist(pkt,ilist,INCLUDE);
	}
	if (elist) {
		if (pkt->p_verbose & DOLIST)
			fprintf(pkt->p_stdout,"Excluded:\n");
		dolist(pkt,elist,EXCLUDE);
	}
	if (glist)
		dolist(pkt,glist,IGNORE);
}
