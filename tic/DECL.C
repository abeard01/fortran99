#include        <stdio.h>
#include        "c.h"
#include        "expr.h"
#include        "gen.h"
#include        "cglbdec.h"
#include        "proto.h"

/*
 *      TMS9900 C Compiler
 *
 *  Copyright 1990 by LGMA Products
 *
 *  This program is free for you to use for non-commercial purposes.
 *  It is, however, copyrighted so that commercial use is strictly
 *  forbidden.
 *
 *  Please contact:
 *
 *     LGMA PRODUCTS
 *     5618 AppleButter Hill Road
 *     Coopersburg, PA  18036
 *
 *  For suggestions, improvements, and the like.
 *
 */

int     imax(i,j)
int     i,j;
{       return (i > j) ? i : j;
}

char    *litlate(s)
char    *s;
{       char    *p;
	p =(char *)xalloc(strlen(s) + 1);
	strcpy(p,s);
	return p;
}

TYP     *maketype(bt,siz)
enum e_bt bt;
int siz;
{       TYP     *tp;
	tp =(TYP *)xalloc(sizeof(TYP));
	tp->val_flag = 0;
	tp->size = siz;
	tp->type = bt;
	tp->sname = 0;
	tp->lst.head = 0;
	return tp;
}

void decl(table)
TABLE   *table;
{        SYM        *spx;  
	 switch (lastst) {
		case kw_char:
			head = tail = maketype(bt_char,1);
			getsym();
			break;
		case kw_void:
			head = tail = maketype(bt_void,0);
			getsym();
			break;
		case kw_short:
		case kw_int:
			head = tail = maketype(bt_short,2);
			getsym();
			break;
		case kw_long:
			head = tail = maketype(bt_long,4);
			getsym();
			break;
		case kw_unsigned:
			getsym();
			if ( lastst == kw_int || lastst == kw_short ) {
				head = tail = maketype(bt_unsigned,2);
				getsym();
			}else if ( lastst == kw_long ) {
				head = tail = maketype(bt_unsigned,4);
				getsym();
			}else if ( lastst == kw_char ) {
				head = tail = maketype(bt_unsigned,1);
				getsym();
			}else{
				head = tail = maketype(bt_unsigned,2);
			}
			break;
		case id:                /* no type declarator */
			spx = gsearch (lastid);
			if ( spx != 0 ) {
			  head = tail = maketype(bt_short,spx->tp->size);
			}else{
			  head = tail = maketype(bt_short,2);
			}
			break;
		case kw_float:
			head = tail = maketype(bt_float,4);
			getsym();
			break;
		case kw_double:
			head = tail = maketype(bt_double,8);
			getsym();
			break;
		case kw_enum:
			getsym();
			declenum(table);
			break;
		case kw_struct:
			getsym();
			declstruct(bt_struct);
			break;
		case kw_union:
			getsym();
			declstruct(bt_union);
			break;
		}
}

void decl1()
{       TYP     *temp1, *temp2, *temp3, *temp4;
	switch (lastst) {
		case id:
			declid = litlate(lastid);
			getsym();
			decl2();
			break;
		case star:
			temp1 = maketype(bt_pointer,2);
			temp1->btp = head;
			head = temp1;
			if(tail == NULL)
				tail = head;
			getsym();
			decl1();
			break;
		case openpa:
			getsym();
			temp1 = head;
			temp2 = tail;
			head = tail = NULL;
			decl1();
			needpunc(closepa);
			temp3 = head;
			temp4 = tail;
			head = temp1;
			tail = temp2;
			decl2();
			temp4->btp = head;
			if(temp4->type == bt_pointer &&
				temp4->val_flag != 0 && head != NULL)
				temp4->size *= head->size;
			head = temp3;
			break;
		default:
			decl2();
			break;
		}
}

void decl2()
{       TYP     *temp1;
	switch (lastst) {
		case openbr:
			getsym();
			temp1 = maketype(bt_pointer,0);
			temp1->val_flag = 1;
			temp1->btp = head;
			if(lastst == closebr) {
				temp1->size = 0;
				getsym();
				}
			else if(head != NULL) {
				temp1->size = constexpr() * head->size;
				needpunc(closebr);
				}
			else {
				temp1->size = constexpr();
				needpunc(closebr);
				}
			head = temp1;
			if( tail == NULL)
				tail = head;
			decl2();
			break;
		case openpa:
			getsym();
			temp1 = maketype(bt_func,0);
			temp1->val_flag = 1;
			temp1->btp = head;
			head = temp1;
			if( lastst == closepa) {
				getsym();
				if(lastst == begin)
					temp1->type = bt_ifunc;
				}
			else
				temp1->type = bt_ifunc;
			break;
		}
}

int     alignment(tp)
TYP     *tp;
{       switch(tp->type) {
		case bt_char:           return AL_CHAR;
		case bt_short:          return AL_SHORT;
		case bt_long:           return AL_LONG;
		case bt_enum:           return AL_SHORT;
		case bt_pointer:
			if(tp->val_flag)
				return alignment(tp->btp);
			else
				return AL_POINTER;
		case bt_float:          return AL_FLOAT;
		case bt_double:         return AL_DOUBLE;
		case bt_struct:
		case bt_union:          return AL_STRUCT;
		default:                return AL_CHAR;
		}
}

int     declare(table,al,ilc,ztype)
/*
 *      process declarations of the form:
 *
 *              <type>  <decl>, <decl>...;
 *
 *      leaves the declarations in the symbol table pointed to by
 *      table and returns the number of bytes declared. al is the
 *      allocation type to assign, ilc is the initial location
 *      counter. if al is sc_member then no initialization will
 *      be processed. ztype should be bt_struct for normal and in
 *      structure declarations and sc_union for in union declarations.
 */
TABLE           *table;
enum e_sc al;
int             ilc;
enum e_bt ztype;
{       SYM     *sp, *sp1;
	TYP     *dhead;
	int     nbytes;
	nbytes = 0;
	decl(table);
	dhead = head;
	for(;;) {
		declid = 0;
		decl1();
		if( declid != 0) {      /* otherwise just struct tag... */
			sp =(SYM *)xalloc(sizeof(SYM));
			sp->name = declid;
			sp->storage_class = al;
			while( (ilc + nbytes) % alignment(head)) {
				if( al != sc_member &&
				    al != sc_external &&
				    al != sc_auto) {
					dseg();
					genbyte(0);
					}
				++nbytes;
				}
			sp->tp = head;
			if( al == sc_static) {
				sp->value.i = (int)nextlabel++;
			}else if( ztype == bt_union) {
				sp->value.i = (int)ilc;
			}else if( al != sc_auto ) {
				sp->value.i = (int)(ilc + nbytes);
/* internal prototype definition */
			}else if( al == sc_auto &&
				  sp->tp->type == bt_func ) {
			      sp->storage_class = sc_external;
			}else{
				sp->value.i =  ilc + nbytes;
			}
			if( sp->tp->type == bt_func && 
				sp->storage_class == sc_global )
				sp->storage_class = sc_external;
			if(ztype == bt_union)
				nbytes = imax(nbytes,(int)(sp->tp->size));
			else if(al != sc_external)
				nbytes += (int)sp->tp->size;
			if( sp->tp->type == bt_ifunc &&
			  (sp1 =(SYM *)search(sp->name,table->head)) != 0 &&
				sp1->tp->type == bt_func )
				{
				sp1->tp = sp->tp;
				sp1->storage_class = sp->storage_class;
				sp1->value.i = sp->value.i;
				sp = sp1;
			}else{
				insert(sp,table);
			}
			if( sp->tp->type == bt_ifunc) { /* function body follows */
				funcbody(sp);
				return nbytes;
				}
			if( (al == sc_global || al == sc_static) &&
				sp->tp->type != bt_func)
				doinit(sp);
			}
		if(lastst == semicolon)
			break;
		needpunc(comma);
		if(declbegin(lastst) == 0)
			break;
		head = dhead;
		}
	getsym();
	return nbytes;
}
int     declbegin(st)
enum e_sym st;
{       return st == star || st == id || st == openpa ||
	       st == openbr;
}

void declenum(table)
TABLE   *table;
{       SYM     *sp;
	TYP     *tp;
	if( lastst == id) {    /* named enum type */
		if((sp = (SYM *)search(lastid,tagtable.head)) == 0) {
			sp = (SYM *)xalloc(sizeof(SYM));
			sp->tp =(TYP *)xalloc(sizeof(TYP));
			sp->tp->type = bt_enum;
			sp->tp->size = 2;
			sp->tp->lst.head = sp->tp->btp = 0;
			sp->storage_class = sc_type;
			sp->name = litlate(lastid);
			sp->tp->sname = sp->name;
			getsym();
			if( lastst != begin)
				error(ERR_INCOMPLETE);
			else    {
				insert(sp,&tagtable);
				getsym();
				enumbody(table);
				}
			}
		else
			getsym();
		head = sp->tp;
	}else{             /* nameless enums */
		tp =(TYP *)xalloc(sizeof(TYP));
		tp->type = bt_short;
		tp->size = 2;
		if( lastst != begin)
			error(ERR_INCOMPLETE);
		else    {
			getsym();
			enumbody(table);
			}
		head = tp;
		}
}

void enumbody(table)
TABLE   *table;
{       int     evalue;
	SYM     *sp;
	evalue = 0;
	while(lastst == id) {
		sp = (SYM *)xalloc(sizeof(SYM));
		sp->value.i =(int)evalue++;
		sp->name = litlate(lastid);
		sp->storage_class = sc_const;
		sp->tp = &stdconst;
		insert(sp,table);
		getsym();
		if( lastst == comma)
			getsym();
		else if(lastst != end)
			break;
		}
	needpunc(end);
}

void declstruct(ztype)
/*
 *      declare a structure or union type. ztype should be either
 *      bt_struct or bt_union.
 */
enum e_bt ztype;
{       SYM     *sp;
	TYP     *tp;
	if(lastst == id) {
		if((sp = (SYM *)search(lastid,tagtable.head)) == 0) {
			sp = (SYM *)xalloc(sizeof(SYM));
			sp->name = litlate(lastid);
			sp->tp =(TYP *)xalloc(sizeof(TYP));
			sp->tp->type = ztype;
			sp->tp->lst.head = 0;
			sp->storage_class = sc_type;
			sp->tp->sname = sp->name;
			getsym();
			if(lastst != begin)
				error(ERR_INCOMPLETE);
			else    {
				insert(sp,&tagtable);
				getsym();
				structbody(sp->tp,ztype);
				}
			}
		else
			getsym();
		head = sp->tp;
		}
	else    {
		tp = (TYP *)xalloc(sizeof(TYP));
		tp->type = ztype;
		tp->sname = 0;
		tp->lst.head = 0;
		if( lastst != begin)
			error(ERR_INCOMPLETE);
		else    {
			getsym();
			structbody(tp,ztype);
			}
		head = tp;
		}
}

void structbody(tp,ztype)
TYP             *tp;
enum e_bt ztype;
{       int     slc;
	slc = 0;
	tp->val_flag = 1;
	while( lastst != end) {
		if(ztype == bt_struct)
			slc += declare(&(tp->lst),sc_member,slc,ztype);
		else
			slc = imax(slc,declare(&tp->lst,sc_member,0,ztype));
		}
	tp->size = (int)slc;
	getsym();
}

void dodecl(defclass)
enum e_sc defclass;
{       struct  ascode  *asnext;

	for(;;) {
	    switch(lastst) {
		case kw_register:
			getsym();
			if( defclass != sc_auto && defclass != sc_member )
				error(ERR_ILLCLASS);
			goto do_decl;
		case id:
			if(defclass == sc_auto)
				return;
/*                      else fall through to declare    */
		case kw_char: case kw_int: case kw_short: case kw_unsigned:
		case kw_long: case kw_struct: case kw_union:
		case kw_enum: case kw_void:
		case kw_float: case kw_double:
do_decl:            if( defclass == sc_global)
			lc_static +=
			    declare(&gsyms,sc_global,lc_static,bt_struct);
		    else if( defclass == sc_auto)
			lc_auto +=
			    declare(&lsyms,sc_auto,lc_auto,bt_struct);
		    else
			    declare(&lsyms,sc_member,0,bt_struct);
/*                            declare(&lsyms,sc_auto,0,bt_struct);  */
		    break;
		case kw_static:
			getsym();
			if( defclass == sc_member)
			    error(ERR_ILLCLASS);
			if( defclass == sc_auto )
			    lc_static += 
			    declare(&lsyms,sc_static,lc_static,bt_struct);
			else
			    lc_static +=
			    declare(&gsyms,sc_static,lc_static,bt_struct);
			break;
		case kw_extern:
			getsym();
			if( defclass == sc_member)
			    error(ERR_ILLCLASS);
			    ++global_flag;
			declare(&gsyms,sc_external,0,bt_struct);
			--global_flag;
			break;

/* if assembly processing keyword asm, then read in the assembly
   statements and just write to the output file                 */
		case kw_asm:
			asnext = asmread();
			swap ( ASMLIST );
			while ( asnext != 0 ) {
				fprintf ( output, asnext->stmt );
				asnext = asnext->fwd;
			}
			swap ( LOCALIST );
			return;

		default:
			return;
		}
	    }
}

void compile()
/*
 *      main compiler routine. this routine parses all of the
 *      declarations using declare which will call funcbody as
 *      functions are encountered.
 */
{       while(lastst != kw_eof) {
		dodecl(sc_global);
		if( lastst != kw_eof)
			getsym();
		}
	dumplits();
}

