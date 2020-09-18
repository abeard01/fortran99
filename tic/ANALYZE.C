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
/* Change History:
**
** Add new revisions to top.
**
**     Who           Date            Reason
**     ===           =========       =======================================
**     ALB           30-Apr-92       Corrected problem with unsigned ints and
**                                   register optimization (v1.62)
 */

/*
 *      this module will step through the parse tree and find all
 *      optimizable expressions. at present these expressions are
 *      limited to expressions that are valid throughout the scope
 *      of the function. the list of optimizable expressions is:
 *
 *              constants
 *              global and static addresses
 *              auto addresses
 *              contents of auto addresses.
 *
 *      contents of auto addresses are valid only if the address is
 *      never referred to without dereferencing.
 *
 *      scan will build a list of optimizable expressions which
 *      opt1 will replace during the second optimization pass.
 */

int equalnode(node1, node2)
/*
 *      equalnode will return 1 if the expressions pointed to by
 *      node1 and node2 are equivalent.
 */
struct enode    *node1, *node2;
{       if( node1 == 0 || node2 == 0 )
                return 0;
        if( node1->nodetype != node2->nodetype )
                return 0;
        if( (node1->nodetype == en_icon || node1->nodetype == en_labcon ||
            node1->nodetype == en_nacon || node1->nodetype == en_autocon) &&
            node1->v.i == node2->v.i )
                return 1;
        if( lvalue(node1) && equalnode(node1->v.p[0], node2->v.p[0]) )
                return 1;
        return 0;
}

struct cse      *searchnode(node)
/*
 *      searchnode will search the common expression table for an entry
 *      that matches the node passed and return a pointer to it.
 */
struct enode    *node;
{       struct cse      *csp;
        csp = olist;
        while( csp != 0 ) {
                if( equalnode(node,csp->exp) )
                        return csp;
                csp = csp->next;
                }
        return 0;
}

struct enode    *copynode(node)
/*
 *      copy the node passed into a new enode so it wont get
 *      corrupted during substitution.
 */
struct enode    *node;
{       struct enode    *temp;
        if( node == 0 )
                return 0;
        temp = (struct enode *)xalloc(sizeof(struct enode));
        temp->nodetype = node->nodetype;
        temp->v.p[0] = node->v.p[0];
        temp->v.p[1] = node->v.p[1];
        return temp;
}

struct  cse  *enternode(node,duse)
/*
 *      enternode will enter a reference to an expression node into the
 *      common expression table. duse is a flag indicating whether or not
 *      this reference will be dereferenced.
 */
struct enode    *node;
int             duse;
{       struct cse      *csp;
        if( (csp = searchnode(node)) == 0 ) {   /* add to tree */
                csp = (struct cse *)xalloc(sizeof(struct cse));
                csp->next = olist;
                csp->uses = 1;
                csp->duses = (duse != 0);
                csp->exp = copynode(node);
                csp->voidf = 0;
                olist = csp;
                return csp;
                }
        ++(csp->uses);
        if( duse )
                ++(csp->duses);
        return csp;
}

struct cse      *voidauto(node)
/*
 *      voidauto will void an auto dereference node which points to
 *      the same auto constant as node.
 */
struct enode    *node;
{       struct cse      *csp;
        csp = olist;
        while( csp != 0 ) {
                if( lvalue(csp->exp) && equalnode(node,csp->exp->v.p[0]) ) {
                        if( csp->voidf )
                                return 0;
                        csp->voidf = 1;
                        return csp;
                        }
                csp = csp->next;
                }
        return 0;
}

void  scanexpr(node,duse,fcall)
/*
 *      scanexpr will scan the expression pointed to by node for optimizable
 *      subexpressions. when an optimizable expression is found it is entered
 *      into the tree. if a reference to an autocon node is scanned the
 *      corresponding auto dereferenced node will be voided. duse should be
 *      set if the expression will be dereferenced.
 */
struct enode    *node;
int             duse, fcall;
{       struct cse      *csp, *csp1;
        if( node == 0 )
                return;
        switch( node->nodetype ) {
                case en_icon:
                case en_labcon:
                case en_nacon:
                        enternode(node,duse);
                        break;
                case en_autocon:
                          if( (csp = voidauto(node)) != 0 ) {
                               csp1 = enternode(node,duse);
                               csp1->uses = (csp1->duses += csp->uses);
                              }
                          else
                              csp1 = enternode(node,duse);
/* if autoconstant node (not passed parameter) used in function call
   as passed address, then void the use of the node (too dangerous)  */
                          if ( node->v.i >= 0        &&
                               fcall     != 0             ) {
                            csp1->voidf = 1;
			  }
                          break;
                case en_v_ref:
                case en_w_ref:
                case en_uw_ref:
                        if( node->v.p[0]->nodetype == en_autocon ) {
				csp = enternode(node,duse);
                                if( csp->voidf )
                                        scanexpr(node->v.p[0],1,fcall);
                                if ( csp1 = searchnode(node->v.p[0]) ) {
                                   if ( csp1->voidf != 0 ) {
                                     csp->voidf = 1;
				   }
				}
			}else{
                                scanexpr(node->v.p[0],1,fcall);
			}
                        break;
                case en_cbl:    case en_cwl:
                case en_cbw:    case en_uminus:
                case en_compl:  case en_ainc:
                case en_adec:   case en_not:
                        scanexpr(node->v.p[0],duse,fcall);
                        break;
                case en_asadd:  case en_assub:
                case en_add:    case en_sub:
                        scanexpr(node->v.p[0],duse,fcall);
                        scanexpr(node->v.p[1],duse,fcall);
                        break;
                case en_mul:    case en_div:
                case en_lsh:    case en_rsh:
                case en_rshl:
                case en_mod:    case en_and:
                case en_or:     case en_xor:
                case en_lor:    case en_land:
                case en_eq:     case en_ne:
                case en_gt:     case en_ge:
                case en_lt:     case en_le:
                case en_asmul:  case en_asdiv:
                case en_asmod:  case en_aslsh:
                case en_asrsh:  case en_asand:
                case en_asor:   case en_cond:
                case en_void:   case en_assign:
                case en_umod:   case en_umul:
                case en_udiv:   case en_ugt:
                case en_uge:    case en_ule:
                case en_ult:
                        scanexpr(node->v.p[0],0,fcall);
                        scanexpr(node->v.p[1],0,fcall);
                        break;
                case en_fcall:
                case en_vfcall:
                        scanexpr(node->v.p[1],0,1);
                        break;
                }
}

void scan(blk)
/*
 *      scan will gather all optimizable expressions into the expression
 *      list for a block of statements.
 */
struct snode    *blk;
{       while( blk != 0 ) {
                switch( blk->stype ) {
                        case st_return:
                        case st_expr:
                                opt4(&blk->exp);
                                scanexpr(blk->exp,0,0);
                                break;
                        case st_while:
                        case st_do:
                                opt4(&blk->exp);
                                scanexpr(blk->exp,0,0);
                                scan(blk->s1);
                                break;
                        case st_for:
                                opt4(&blk->label);
                                scanexpr(blk->label,0,0);
                                opt4(&blk->exp);
                                scanexpr(blk->exp,0,0);
                                scan(blk->s1);
                                opt4(&blk->s2);
                                scanexpr(blk->s2,0,0);
                                break;
                        case st_if:
                                opt4(&blk->exp);
                                scanexpr(blk->exp,0,0);
                                scan(blk->s1);
                                scan(blk->s2);
                                break;
                        case st_switch:
                                opt4(&blk->exp);
                                scanexpr(blk->exp,0,0);
                                scan(blk->s1);
                                break;
                        case st_case:
                                scan(blk->s1);
                                break;
                        case st_default:
                                scan(blk->s1);
                                break;
                        }
                blk = blk->next;
                }
}

void exchange(c1)
/*
 *      exchange will exchange the order of two expression entries
 *      following c1 in the linked list.
 */
struct cse      **c1;
{       struct cse      *csp1, *csp2;
        csp1 = *c1;
        csp2 = csp1->next;
        csp1->next = csp2->next;
        csp2->next = csp1;
        *c1 = csp2;
}

int     desire(csp)
/*
 *      returns the desirability of optimization for a subexpression.
 *      ignore this subexpression if integer constant or if it has been
 *      voided.
 */
struct cse      *csp;
{     
/*        if( csp->voidf || (csp->exp->nodetype == en_icon &&
                        csp->exp->v.i < 16 && csp->exp->v.i >= 0))
                return 0;         */
        if ( csp->voidf || csp->exp->nodetype == en_icon ) {
                return 0;
	}
        if( lvalue(csp->exp) )
                return 2 * csp->uses;
        return csp->uses;
}

int     bsort(lst)
/*
 *      bsort implements a bubble sort on the expression list.
 */
struct cse      **lst;
{       struct cse      *csp1, *csp2;
	csp1 = *lst;
        if( csp1 == 0 || csp1->next == 0 )
                return 0;
	bsort( &(csp1->next));
        csp2 = csp1->next;
        if( desire(csp1) < desire(csp2) ) {
                exchange(lst);
                return 1;
                }
        return 0;
}

void allocate()
/*
 *      allocate will allocate registers for the expressions that have
 *      a high enough desirability.
 */
{       struct cse      *csp;
        struct enode    *exptr;
        int             datareg;
        struct amode    *ap;

        datareg = 4;
        while( bsort(&olist) );         /* sort the expression list */
        csp = olist;
        while( csp != 0 ) {
                if( desire(csp) < 2 )	/* also try < 3 */
                        csp->reg = -1;
                else if( datareg < 7 )
                        csp->reg = datareg++;
                else
                        csp->reg = -1;
                csp = csp->next;
                }
        csp = olist;
        while( csp != 0 ) {
                if( csp->reg != -1 ) {	/* see if preload needed */
                        exptr = csp->exp;
                        initstack();
/* if ref following by autoconstant, then if positive autoconstant
   (like @2(R7)) then eliminate preload, register takes place of
   variable.								*/
                        if (  (exptr->nodetype == en_w_ref ||
                               exptr->nodetype == en_uw_ref)  &&
                              exptr->v.p[0]->nodetype == en_autocon ) {

                           if ( exptr->v.p[0]->v.i < 0 ) {
                              ap = gen_expr(exptr,F_ALL,2);
                              gen_loadr(ap, 2, csp->reg);
                              freeop(ap);
			   }
			}else{
                          ap = gen_expr(exptr,F_ALL,2);
                          gen_loadr ( ap, 2, csp->reg );
                          freeop(ap);
			}
		}
         csp = csp->next;
         }
}

void  repexpr(node)
/*
 *      repexpr will replace all allocated references within an expression
 *      with tempref nodes.
 */
struct enode    *node;
{       struct cse      *csp;
        if( node == 0 )
                return;

        switch( node->nodetype ) {
                case en_icon:
                case en_nacon:
                case en_labcon:
                case en_autocon:
                      if( (csp = searchnode(node)) != 0 )
                                if( csp->reg > 0 ) {
                                        node->nodetype = en_tempref;
                                        node->v.i = csp->reg;
                                        }
                        break;
                case en_b_ref:
                case en_ub_ref:
                case en_v_ref:
                case en_w_ref:
                case en_uw_ref:
                case en_l_ref:
                case en_ul_ref:
                        if( (csp = searchnode(node)) != 0 ) {
                                if( csp->reg > 0 ) {
                                        node->nodetype = en_tempref;
                                        node->v.i = csp->reg;
                                        }
                                else
                                        repexpr(node->v.p[0]);
                                }
                        else
                                repexpr(node->v.p[0]);
                        break;
                case en_cbl:    case en_cbw:
                case en_cwl:    case en_uminus:
                case en_not:    case en_compl:
                case en_ainc:   case en_adec:
                        repexpr(node->v.p[0]);
                        break;
                case en_add:    case en_sub:
                case en_mul:    case en_div:
                case en_mod:    case en_lsh:
                case en_rsh:    case en_and:
                case en_rshl:
                case en_or:     case en_xor:
                case en_land:   case en_lor:
                case en_eq:     case en_ne:
                case en_lt:     case en_le:
                case en_gt:     case en_ge:
                case en_cond:   case en_void:
                case en_asadd:  case en_assub:
                case en_asmul:  case en_asdiv:
                case en_asor:   case en_asand:
                case en_asmod:  case en_aslsh:
                case en_asrsh:  case en_fcall:
                case en_assign:
                case en_vfcall:
                case en_umod:   case en_umul:
                case en_udiv:   case en_ult:
                case en_ule:    case en_uge:
                case en_ugt:
                        repexpr(node->v.p[0]);
                        repexpr(node->v.p[1]);
                        break;
                }
}

void repcse(blk)
/*
 *      repcse will scan through a blk of statements replacing the
 *      optimized expressions with their temporary references.
 */
struct snode    *blk;
{       while( blk != 0 ) {
                switch( blk->stype ) {
                        case st_return:
                        case st_expr:
                                repexpr(blk->exp);
                                break;
                        case st_while:
                        case st_do:
                                repexpr(blk->exp);
                                repcse(blk->s1);
                                break;
                        case st_for:
                                repexpr(blk->label);
                                repexpr(blk->exp);
                                repcse(blk->s1);
                                repexpr(blk->s2);
                                break;
                        case st_if:
                                repexpr(blk->exp);
                                repcse(blk->s1);
                                repcse(blk->s2);
                                break;
                        case st_switch:
                                repexpr(blk->exp);
                                repcse(blk->s1);
                                break;
                        case st_case:
                                repcse(blk->s1);
                                break;
                        case st_default:
                                repcse(blk->s1);
                                break;
                        }
                blk = blk->next;
                }
}

void opt1(blk)
/*
 *      opt1 is the externally callable optimization routine. it will
 *      collect and allocate common subexpressions and substitute the
 *      tempref for all occurrances of the expression within the block.
 */
struct snode    *blk;
{
        olist = 0;
        scan(blk);                /* collect expressions */
	if ( regflag )
           allocate();            /* allocate registers */
        repcse(blk);              /* replace allocated expressions */
}
