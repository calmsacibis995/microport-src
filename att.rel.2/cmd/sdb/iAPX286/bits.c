/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*   bits.c: 1 3/16/84        */
/*	dissaembler for the iapx80286 sdb
 *	
 */
#include	<stdio.h>
#include	<sys/reg.h>
#include	"head.h"
#include	"dis.h"

int errlev;	/* To keep track of errors encountered during  */
		/* the disassembly, probably due to being out  */
		/* of sync.				       */
/* the following tables are in tbls.c 			       */
extern struct optab optab[];  /*  opcode mnemonics,functions,flags */
extern struct optab optabba[];/*  opcode breakout table A          */
extern struct optab optabb1a[];/* opcode breakout table A1	   */
extern struct optab optabbb[];/*  opcode breakout table b          */
extern struct optab optabbc[];/*  opcode breakout table C          */
extern struct optab optabbd[];/*  opcode breakout table D          */
extern struct optab optabbr[];/*  opcode breakout table for shifts */
extern struct optab optabfa[];/*  opcode breakout table float point*/
extern struct optab optabfb[];/*  opcode breakout table float point*/
extern struct optab optabfc[];/*  opcode breakout table float point*/
extern struct optab optabws[];/*  opcode for wait specials	   */
extern char *segovtbl[];      /*  segment overide table		   */
extern char *reglistd[];      /*  register print templates          */
extern long dispaddr;	      /*  displacement address for symbol  */
extern char decodetype;	      /*  decode type			   */
extern int override;	      /*  override es,ds,cs,ss             */
extern long loc;	      /*  byte location being disassembled */
extern char mneu[];	      /*  line to be printed if required   */
extern int argument[];        /*  array of operands                */
extern char *curpos;          /*  pointer to the sub entry in print line */

char dis_bget() ;

/*  dis_dot attempts to disassemble an instruction, and returns the
 *       location of the next instuction
 */


long
dis_dot(ldot, idsp, fmt)
long ldot;
int idsp;
char fmt;	/* not used */
{
	         struct optab * ip; /*  pointer to the opcode table entry */
		 	char  * segov;  /*  segment overide		  */
        		int     displ;  /*  displacement from base	  */
               		int     imval;  /*  immediate value		  */
			int	segovcnt; /* mechenism to stop indef. loop*/
			int	work;	/*  work area for temp variables  */
			long	errsave; /* save the address for error ret*/
			unsigned char    a1; /*  first byte of instruction*/
/*    	Get the opcode of the current instruction			  */
	loc = ldot;     /*  set the return pointer to the user start      */
	segov = "";    /*  set the seg. overide display to null          */
	segovcnt = 0;   /*  set the loop guard to initial state if > 1 loop */
	errlev = 0;	/*  set the error lev to zero if > 0 error	  */
	decodetype = NILL; /*  not local or param or label or external    */
	override = NONE; /*  default override depends on context	  */
	displ=imval=0;
	argument[3]=0; /* reset the 3rd address in case prior was a triple */
segovl:
	a1=(dis_bget(loc++,idsp)&BYTEMASK); 
	ip = &optab[a1]; 
	errsave = loc;
 	argument[2] = (int)( ip -> iflag & BYTEMASK );
	switch ( ip -> iformat & BYTEMASK ) {	
	case 0: /* illegal instruction [ILLEGAL] */
		errlev++;
		return(loc);
	case 1: /* mod reg rm format   [MODREGRM] */
		displ = dis_dec(idsp);
		break;
	case 2: /* immediate operand   [IMM]      */
		imval = dis_imm(idsp);
		argument[0] = ((ip -> iflag >> SEGSHIFT) & ARGMASK);
		argument[1] = IMMEDIATE;
		break;
	case 3: /* segment register   [SEGR]      */
		argument[0] = (( ip -> iflag >> SEGSHIFT ) & ARGMASK );
		argument[1] = NOARG;
		break;
	case 4: /* segment overide    [SEGOVR]    */
		if ( segovcnt++ == 0 ) {
			override = (ip -> iflag & ARGMASK) - SDBES;
  			segov = segovtbl[ override ];
			goto segovl;
		}
		else {
			errlev++;
			return(loc);
		}
	case 0x1a: /* wait check for special floating point combinations */
		if ((dis_bget(loc,idsp)&BYTEMASK) == 0xdb)
		{
			a1 = dis_bget(++loc,idsp);
			loc--;
			if((a1==0xe2)||(a1==0xe3)||(a1==0xe4))
			{
			loc += 2;
			ip = &optabws[a1-0xe2];
			}
		}
	case 5: /* no arguments single byte instruction [SINGLE] */
		argument[0] = argument[1] = NOARG;
		break;
	case 6: /* short jumps  and long relative to new pc    [DISP]    */
		decodetype = LABEL;
		displ = dis_imm(idsp);
		argument[0] = DISPONLY;
		argument[1] = NOARG;
		break;
	case 7: /* immediate that can be sign extended [IMM1] */
		imval = dis_imm(idsp);
		argument[0] = NOARG;
		argument[1] = IMMEDIATE;
		break;
	case 8: /* register with accumulator   [XCHG]    */
		argument[0] = SDBAX;
		argument[1] = ((ip -> iflag) >> SEGSHIFT & REGMASK);
		break;
	case 9: /* segment registers to and from memory [MODREGRMS] */
		displ = dis_dec(idsp);
		/*  adjust the reg argument to the seg registers  */
		argument[0] = argument[0] + SDBES ;
		break;
	case 0xa: /* load register from memory only [MODREGRMM] */
		displ = dis_dec(idsp);
		/* check that memory access only  */
		if ((argument[1] < SDBXSD) || (argument[1] > SDBD)) {
			errlev++;
			return(loc);
		}
		break;
	case 0xb: /* rm only REG = 0 [MODREGRMRM] */
   		displ = dis_dec(idsp);
		if (argument[0] != 0) {
			errlev++;
			return(loc);
		}
		/* check that memory access only */
		if ((argument[1] < SDBXSD) || (argument[1] > SDBD)) {
			errlev++;
			return(loc);
		}
		argument[0] = NOARG;
		break;
	case 0xc: /* immediate on rm [MODREGRMI]  */
		displ = dis_dec(idsp);
		if ((argument[0] > 0) && (argument[0] < 8)) {
			errlev++;
			return(loc);
		}
		argument[0] = argument[1];
		argument[1] = IMMEDIATE;
		imval = dis_imm(idsp);
		break;
	case 0xd: /*  indirect via seg   [DIRISEG]  */
		decodetype = LABELL;
		displ = dis_wget(idsp);
		imval = dis_wget(idsp);
		dispaddr = (((long)imval)<<16) + (unsigned)displ;
		argument[0] = SDB2;
		argument[1] = SEGMI;
		break;
	case 0xe: /*  check the second byte for 0xa [CHECKB] */
		if (dis_bget(loc++,idsp) != 0xa) {
			errlev++;
			return(loc);
		}
		argument[0] = argument[1] = NOARG;
		break;
	case 0xf:  /*  put dx in the reg field [INSDX]  */
		argument[0] = DXADDR;
		argument[1] = ((ip -> iflag >> SEGSHIFT) & ARGMASK);
		break;
	case 0x10: /*  memory to acc or vice versa [MEMACC]  */
		decodetype = EXTERN;
		displ = dis_wget(idsp);
		argument[0] = ((ip -> iflag >> SEGSHIFT) & ARGMASK);
		argument[1] = SDBD;
		break;
	case 0x11: /*  entry [IMM2]  */
		displ = dis_wget(idsp);
		imval = dis_bget(loc++,idsp);
		argument[0] = SDB2;
		argument[1] = IMMEDIATE;
		break;
   	case 0x12:  /*  breakout a for 0x0f 1st byte [BREAKA]  */
		if ((work = dis_bget(loc++,idsp) & BYTEMASK) > 6) {
			errlev++;
			return(loc);
		}
		ip = &optabba[work];
		argument[0] = NOARG;
		switch (ip -> iformat & BYTEMASK) {
		case 0 : /*  illegal instruction  */
			errlev++;
			return(loc);
		case 1 : /*  single byte instr type */
			argument[1] = NOARG;
			break;
		case 2 : /*  normal decode  [MODREGRM] type */
			argument[2] = (ip -> iflag & BYTEMASK);
			displ = dis_dec(idsp);
			break;
		case 3 : /*  reg field is futher decoded   */
			argument[2] = (ip -> iflag & BYTEMASK);
			displ = dis_dec(idsp);
/*  argument 1 now messed up if byte correct  */
			if ((argument[1] > 7 ) && ( argument[1] < 16)) {
 				argument[1] -= 8;
			}
			ip = &optabb1a[argument[0]];
			argument[2] = (ip -> iflag & BYTEMASK);
			if (ip -> iformat & BYTEMASK == 0) {
				errlev++;
				return(loc);
			}
			if ((argument[2]&MEMO)&&((argument[1]<SDBXSD) || (argument[1]>SDBD))) {
				errlev++;
				return(loc);
			}
			argument[0] = NOARG;
			break;
		}  /* case end */
		break;
	case 0x13:  /*  breakout b arith and logical lit [BREAKB] */
		displ = dis_dec(idsp);
		argument[0] += (argument[2]&SHIFTMASK);
		ip = &optabbb[argument[0]];
		if (ip -> iformat == 0) {
			errlev++;
			return(loc);
		}
		argument[0] = argument[1];
		if (((~ip->iflag)&SWORD)&&(argument[0]< 8))
		{
				argument[0] += 8;
		}	
		argument[1] = IMMEDIATE;
		argument[2] = (ip -> iflag & BYTEMASK);
		imval = dis_imm(idsp);
		break;
	case 0x14:  /*   inc call etc with reg/memory [BREAKC]   */
		displ = dis_dec(idsp);
		/*  reg in argument[0] is range 0-7 for word and 8-15 for byte*/
		ip = &optabbc[argument[0]];
		argument[0] = NOARG;
		if (ip -> iformat == ILLEGAL) {
			errlev++;
			return(loc);
		}
		if((ip -> iflag & MEMO) && ((argument[1] < SDBXSD) || (argument[1] > SDBD))) {
			errlev++;
			return(loc);
		}
		break;
	case 0x15:  /*  breakd    [BREAKD]  */
		displ = dis_dec(idsp);
		ip = &optabbd[argument[0]];
		argument[2] = (ip -> iflag & BYTEMASK);
		switch ( ip -> iformat & BYTEMASK ) {
		case 0: /*  illegal instruction  */
			errlev++;
			return(loc);
		case 1: /*  no immediate data  */
			argument[0] = NOARG;
			break;
		case 2: /*  immediate data  */
			argument[0] = argument[1];
			argument[1] = IMMEDIATE;
			imval = dis_imm(idsp);
		} /* end of case */
		break;
	case 0x16:  /*  shift instructions */
		displ = dis_dec(idsp);
		ip = &optabbr[argument[0]];
		if ( ip -> iformat == ILLEGAL ) {
			errlev++;
			return(loc);
		}
		argument[0] = argument[1];
		argument[1] = ((((argument[2] >> SEGSHIFT)& ARGMASK ) << 1)+1);
		if ( argument[1] == 1 )  { 
			argument[1] = IMMEDIATE;
			imval = dis_bget(loc++,idsp);
		}	
		break;
	case 0x17: /* triple address mode [TRIPLE]  */
		displ = dis_dec(idsp);
		imval = dis_imm(idsp);
		argument[3] = argument[1];
		argument[1] = TRIPLEA; /* this will get swapped because of IM */
		break;
	case 0x18: /* D8 floating point format ESCD8 */
		displ = dis_dec(idsp);
		argument[3] = FLOATPOINT;
     		ip = &optabfa[(((argument[0]<<3)&0x38)+(((int)(a1)>>1)&0x03)+(argument[1]>8?0:4))];
		argument[2] =((a1&4)?RTORM:RMTOR);
		argument[0] = SDBST;  /* st  stack top  */
		switch ( ip -> iformat & BYTEMASK ) {
		case 0: /*  illegal instruction  */
			errlev++;
			return(errsave);
		case 3: /*  comp command but executable */
			errlev++;
		case 4: /*  memmory operation    */ 
		case 2: /*  comp comand          */
			argument[2] = RMTOR;
			argument[0] = SDBI;  /* use only one argument */
			break;
		case 1: /*  reserved instruction */
			errlev++;
		case 5: /*  fcompp has no arguments */
			argument[0] = argument[1] = SDBI;
			break;
		}
		break;
	case 0x19: /* ESC floating point format the rest */
		/* read the next byte and do as the 286 does to determin the */
		/* fact that it is memory operand check top two bits to see  */
		/* if both are set					     */
		work = dis_bget(loc,idsp);
		argument[3] = FLOATPOINT;
		argument[2] = RMTOR;
		argument[0] = SDBI;  /* use only one argument */
		if (((work>>6)&3) == 3) 
		{
			/* register register operation? */
			loc++;
			argument[1] = work & 0x7;
			ip = &optabfb[((((a1<<5)&0xc0)+(work&0x3f))&BYTEMASK)];
			switch ( ip -> iformat & BYTEMASK ) {
			case 0: /* illegal     */
				errlev++;
				return(errsave);
			case 1: /* reserved    */
				argument[1] = SDBI; /* no arguments */
				/* KLUDGE ALERT
				   special case for fstsw %ax
				   its frigged so that it lloks like a reserved
				   but produces fstsw %ax as the mnemonic field
				   so don't save loc */
				if(strcmp(ip->iname,"fstsw	%ax"))
					loc = errsave;
				break;
			case 2: /* no arguments */
				argument[1] = SDBI; /* no srguments */
				break;
			case 3: /* register arguments */
				break;
			case 4: /* executes but not generated by the compiler*/
				errlev++;
				break;
			}
		}
		else
		{
		/* this is a memory operation */
			displ = dis_dec(idsp);
			argument[0] = SDBI;
			ip = &optabfc[(((a1>>1)&3) + ((work>>1)&0x1c))];
			switch ( ip -> iformat & BYTEMASK ) {
			case 0: /* illegal     */
				errlev++;
				return(errsave);
			case 1: /* reserved    */
				argument[1] = SDBI; /* no arguments */
				loc = errsave;
				break;
			}
		}
		break;
	} /* case */
/*  print the decoded line in the mneu print line  */
sprintf(mneu,"%-8s",ip -> iname);
curpos = mneu + 8;
if (argument[2]&RMTOR) {
	work = argument[0];
	argument[0] = argument[1];
	argument[1] = work;
}
if (argument[3] == FLOATPOINT) {
	if (argument[0] != SDBI)
	{
		dis_fpr(argument[0],segov,displ);
		if (argument[1] != SDBI)
		{
			curpos += sprintf(curpos,",");
		}
	}
        if (argument[1] != SDBI)
	{
		dis_fpr(argument[1],segov,displ);
	}
}
else {
	if (argument[0] != NOARG)
	{
		dis_pr(argument[0],segov,displ,imval);
		if (argument[1] != NOARG)
		{
			curpos += sprintf(curpos,",");
		}
	}
        if (argument[1] != NOARG)
	{
		dis_pr(argument[1],segov,displ,imval);
	}
}
/*  now try to produce an absolute address for the symbolic section */
	if ((fmt=='\0')||(((struct user *)uu)->u_ar0==NULL)||decodetype==(LABELL))
		return(loc);
	dispaddr = ((long)((unsigned)displ));
	if (decodetype == NILL)
		return(loc);
	if (decodetype == LABEL)
	{
		dispaddr = ((dispaddr + loc)&0xffff) + (((long)SDYREG(CS))<<16);
		return(loc);
	}
	if (argument[0] > SDBBH && argument[0] < SDBI)
	{
		dis_getaddr(argument[0]);
		return(loc);
	}
	dis_getaddr(argument[1]);
	return(loc);
};
char
dis_bget(loctn,idsp)   /* get a byte at location */
long loctn;
int idsp;
{
char a;
a=(char)(chkget(loctn,idsp)&BYTEMASK);
return(a);
/*    return((char)(chkget(loctn,idsp)>>8)&BYTEMASK);  */
}
int
dis_wget(idsp)  /*  get a word from the system */
int idsp;
{
return((dis_bget(loc++,idsp)<<8)+(dis_bget(loc++,idsp)&BYTEMASK));
}
int
dis_imm(idsp)  /* this routine looks at the flags and gets a word byte or sign*/		/* extended byte returned as int */
int idsp;
{
if (argument[2] & SWORD) {
	if (argument[2] & SE) {
		return((dis_bget(loc++,idsp)<<8)>>8);
	}
	else {
		return(dis_wget(idsp));
	}
}
else {
	return(dis_bget(loc++,idsp)&BYTEMASK);
}
}
/*  This routine decodes the mod reg rm byte and returns the displacement
 *   	if there is one else zero. It also sets the appropriate arguments
 * 	that reflect a route through the register combinations
 */
int
dis_dec(idsp)  /*  decode the mod reg rm byte */
int idsp;
{
int ea;    /*  the effective address byte held as int */
int mod;   /*  the mode of operation                  */
int reg;   /*  the main reg configuration             */
int rm;    /*  register or memory depending on mode   */
int disp;  /*  item for return			      */
ea = dis_bget(loc++,idsp);  /* get the effective address byte */
mod = (ea>>MODSHIFT)&0x03;
reg = (ea>>REGSHIFT)&REGMASK;
rm  = ea&REGMASK;
decodetype = EXTERN;
disp = 0;
argument[0]=argument[1]=0xff;
switch(mod) {
	case 0: /*  disp = 0 unless rm is 6 then disp is 2byte disp */
    		if ( rm == 6 ) {
			disp = dis_wget(idsp);
			argument[1]=DISPONLY;
		}
		else {
			argument[1] = RMBASE + rm;
		}
		break;
	case 1: /*  sign extend single byte disp */
		disp = dis_bget(loc++,idsp); /* does this auto sign extend */  
		argument[1] = RMBASE + rm;
		break;
	case 2: /*  full 2byte displacement  */
		disp = dis_wget(idsp);
		argument[1] = RMBASE + rm;
		break;
	case 3: /*  this is where rm is treated as a register  */
		if (argument[2]&SWORD ) {
			argument[1] = rm;
		}
		else {
			argument[1] = rm + REGLSTS;
		}
		break;
	}
	/*  deal with the reg field in argument[0]  */
if ( argument[2]&SWORD ) {
	argument[0] = reg;
}
else {
	argument[0] = reg + REGLSTS;
}
return(disp);
}
dis_pr(mode,segov,displ,imval)
int mode;
char *segov;
int displ;
int imval;
{
int work;
triplesw:
switch (mode) {
case SDBAX:
case SDBCX:
case SDBDX:
case SDBBX:
case SDBSP:
case SDBBP:
case SDBSI:
case SDBDI:
case SDBAL:
case SDBCL:
case SDBDL:
case SDBBL:
case SDBAH:
case SDBCH:
case SDBDH:
case SDBBH:
case SDBES:
case SDBCS:
case SDBSS:
case SDBDS:
case PRINT1:
case DXADDR:
/* all register no overide */
	curpos += sprintf(curpos,"%s",reglistd[mode]);
	break;
/* triple address put out the immediate value, then jumps to deal with the ew */
case TRIPLEA:
	mode = argument[3];
	argument[3]=TRIPLEA; /* this shows it did happen */
  	curpos += sprintf(curpos,"$0x%1x,",(imval & 0xffff));
	goto triplesw;
case SDBXSD:
case SDBXDD:
case SDBPID:
case SDBPDD:
case SDBSD:
case SDBDD:
case SDBXD:
case SDBD:
/* memory mode with or without displacement */
/* there may be a posible overide           */
	curpos += sprintf(curpos,"%s0x%0x%s",segov,(displ & 0xffff),reglistd[mode]);
	break;
case SDBPD:
/* put out the displacement in decimal if based on bp */
	curpos += sprintf(curpos,"%s%d%s",segov,displ,reglistd[mode]);
	break;
case SDB2:
/* two words to print first the displacement then segment(printed the second*/
/* time through as a literal*/
	curpos += sprintf(curpos,"$0x%1x",(displ & 0xffff));
	break;
case SDBI:
	curpos += sprintf(curpos,"$0x%1x",(imval & 0xffff));
	break;
case SEGMI:
        curpos += sprintf(curpos,"0x%1x",(imval & 0xffff));
} /* end of case */
return(0);
}
dis_fpr(mode,segov,displ)
int mode;
char *segov;	
int displ;
{
	switch ( mode ) {
	case SDBAX:
	case SDBCX:
	case SDBDX:
	case SDBBX:
	case SDBSP:
	case SDBSI:
	case SDBBP:
	case SDBDI:
		curpos += sprintf(curpos,"%%st(%d)",mode);
		break;
	case SDBST:
		curpos += sprintf(curpos,"%%st");
		break;
	case SDBXSD:
	case SDBXDD:
	case SDBPID:
	case SDBPDD:	
	case SDBSD:
	case SDBDD:
	case SDBXD:
	case SDBD:
		curpos += sprintf(curpos,"%s0x%0x%s",segov,(displ & 0xffff),reglistd[mode]);
		break;
	case SDBPD:
	/* put out the displacement in decimal if based on bp */
		curpos += sprintf(curpos,"%s%d%s",segov,displ,reglistd[mode]);
		break;
	} /* case end */
return(0);
}
dis_getaddr(decode)
{
	if (decode < SDBXSD || decode > SDBD)
	{
		decodetype = NILL; /* register operation */
		return;
	}
	switch(decode) {
	case SDBXSD:
		dispaddr += ((long)SDYREG(BX) + (long)SDYREG(SI));
		break;
	case SDBXDD:
		dispaddr += ((long)SDYREG(BX) + (long)SDYREG(DI));
		break;
	case SDBPID:
		dispaddr += (long)SDYREG(SI);
		goto ssbase;
	case SDBPDD:
		dispaddr += (long)SDYREG(DI);
		goto ssbase;
	case SDBSD:
		dispaddr += (long)SDYREG(SI);
		break;
	case SDBDD:
		dispaddr += (long)SDYREG(DI);
		break;
	case SDBPD:
		goto ssbase;
	case SDBXD:
		dispaddr += (long)SDYREG(BX);
		break;
	} /* end of the case statement */
	dispaddr = dispaddr&0xffff; /* keep only disp and add segment */
	if (override == NONE)
	{
		dispaddr += (((long)SDYREG(DS))<<16);
		return;
	}
ssoverride:
	switch ( override ) {
	case ESO:
		dispaddr += (((long)SDYREG(ES))<<16);
		return;
	case CSO:
		dispaddr += (((long)SDYREG(CS))<<16);
		return;
	case SSO:
		dispaddr += (((long)SDYREG(SS))<<16);
		decodetype = NILL;
		return;
	case DSO:
		dispaddr += (((long)SDYREG(DS))<<16);
		return;
	} /* end of the override case */
	return;
ssbase:
	if (override == NONE || override == SSO)
	{
		if (dispaddr > 0) 
		{
			decodetype = SPARAM;
			return;
		}
		decodetype = SLOCAL;
		return;
	}
	dispaddr += ((long)SDYREG(BP));
	dispaddr = dispaddr & 0xffff;
	goto ssoverride;
}	
