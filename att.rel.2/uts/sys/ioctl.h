/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)ioctl.h	1.6 */

/*
**	Union for use by all device handler ioctl routines.
*/
union ioctl_arg {
	struct termio	*stparg;	/* ptr to termio struct */
	struct Generic	*sparg;		/* ptr to generic struct */
	char		*cparg;		/* ptr to character */
	char		carg;		/* character */
	int		*iparg;		/* ptr to integer */
	int		iarg;		/* integer */
};
