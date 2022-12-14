/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)list.h	1.3 - 85/08/09 */
/*
 */

#define l_AI 1		/* list of ACTITEMs			*/
#define l_IF 2		/* list of Input Files (INFILIST)	*/
#define l_IS 3		/* list of Input Sections (INSEC List)	*/
#define l_OS 4		/* list of Output Sections (OUTSEC List) */
#define l_INC 5		/* list of input sections included in an
				output sect			*/
#define l_REG 6		/* list of REGIONs			*/
#define l_MEM 7		/* list of MEMTYPEs			*/
#define l_ADR 8		/* list of ANODEs off of REGION		*/
#define l_DS 9		/* list of dummy output sections (DSECT list) */
#define l_GRP 10	/* list of output sections in a "group"	*/
#define l_CM 11		/* list of pseudo COMMON sections in ifile */
