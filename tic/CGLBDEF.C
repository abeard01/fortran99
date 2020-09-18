#include        <stdio.h>
#include        "c.h"
#include        "expr.h"
#include        "gen.h"
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

/*      global definitions      */

FILE            *input = 0,
                *list = 0,
                *copy = 0,
                *output = 0;

int             lineno = 0;
int             nextlabel = 0;
char            lastch = 0;
int             lastst = 0;
char            lastid[20] = "";
char		laststr[MAX_STLP1] = "";
int             ival = 0;
/* double          rval = 0.0;				*/

TABLE           gsyms = {0,0},
                lsyms = {0,0};
SYM             *lasthead = NULL;
struct slit     *strtab = 0;
int             lc_static = 0;
int             lc_auto = 0;
struct snode    *bodyptr = 0;
int             global_flag = 1;
TABLE           defsyms = {0,0};
int             save_mask = 0;          /* register save mask */
int		optflag   = 1;		/* optimization flag  */
char            infile[MAX_FILE],	/* input file */
                listfile[MAX_FILE],	/* listing file */
                outfile[MAX_FILE],	/* output file */
                copyfile[MAX_FILE],     /* copy file */
		incfile[MAX_FILE];	/* include file */
int		mainflag;
int             optimized;		/* optimized this pass or not */
int             listflag = 1;           /* listing ON */
int             copyflag = 0;           /* copy flag OFF */
char            inc_files[MAX_INCLUDE];	/* storage for include names  */
char            src_files[MAX_FILE];    /* storage for source redirect */
char            *incptr;		/* include pointer */
char            *srcptr;                /* source redirect pointer */
int		total_errors = 0;	/* total accumulated errors */
char		inline[132];		/* input line */
int		numerrs = 0;		/* number of errors */
int             errnos[MAX_ERROR];      /* errors */

char		*linstack[20];		/* stack for substitutions */
char		chstack[20];		/* place to save lastch */
int		lstackptr = 0;		/* substitution stack ponter */
struct cse      *olist;			/* list of optimizable expressions */
TYP             *head = 0;
TYP             *tail = 0;
char            *declid = 0;
TABLE           tagtable = {0,0};
TYP             stdconst =  { bt_short, 1, 2, {0, 0}, 0, "stdconst"};
TYP             stdint =    { bt_short, 0, 2, {0, 0}, 0, 0 };
TYP             stduint =   { bt_unsigned, 0, 2, {0,0}, 0, 0 };
TYP             stdchar =   { bt_char, 0, 1, {0, 0}, 0, 0 };
TYP             stdstring = { bt_pointer, 1, 2, {0, 0}, &stdchar, 0};
TYP             stdfunc =   { bt_func, 1, 0, {0, 0}, &stdint, 0};
int             breaklab;
int             contlab;
int             retlab;
int             glbsize = 0;    /* size left in current global block */
int		mansize = 0;	/* size left in current peepgen block */
int             locsize = 0;    /* size left in current local block */
int		asmsize = 0;	/* size left in assembly local block */
int             glbindx = 0;    /* global index */
int		manindx = 0;	/* peepgen index */
int             locindx = 0;    /* local index */
int		asmindx = 0;	/* assembly index */
struct blkmem   *locblk = 0;    /* pointer to local memory block */
struct blkmem   *manblk = 0;	/* pointer to peepgen memory block */
struct blkmem   *glbblk = 0;    /* pointer to global memory block */
struct blkmem   *asmblk = 0;	/* pointer to assembly memory block */
int             outcol = 0;
struct ocode    *peep_head = 0;
struct ocode    *peep_tail = 0;
FILE            *inclfile[MAX_FILE];
int             incldepth = 0;  /* pre-processor use */
int             inclline[10];
int		ifdepth = 0;	/* if depth */
int		ifarray[10];	/* if array */
char            *lptr;
int             next_data;	/* used for register processing */
int             max_data;
int             regflag;        /* register optimization flag */
int             windowflag;	/* window output usage flag */
int             manlis;		/* which list am I managing (0=loc, 1=peep) */
int             quiet;          /* quiet compilation flag */
int		assembly = 0;	/* processing assembly code (1) or not (0) */
