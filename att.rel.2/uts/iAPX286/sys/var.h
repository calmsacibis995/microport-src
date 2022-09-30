/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)var.h	1.5 */
struct var {
	int	v_buf;
	int	v_call;
	int	v_inode;
	char *	ve_inode;
	int	v_file;
	char *	ve_file;
	int	v_mount;
	char *	ve_mount;
	int	v_proc;
	char *	ve_proc;
	int	v_text;
	char *	ve_text;
	int	v_clist;
	int	v_sabuf;
	int	v_maxup;
	int	v_smap;
	int	v_hbuf;
	int	v_hmask;
	int	v_pbuf;
	int	v_cmap;
	int	v_autoup;
};
extern struct var v;
