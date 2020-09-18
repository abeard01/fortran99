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

/*      global declarations     */

extern FILE             *input,
                        *list,
                        *copy,
                        *output;

extern int              lineno;
extern int              nextlabel;
extern char             lastch;
extern enum e_sym       lastst;
extern char             lastid[20];
extern char             laststr[MAX_STLP1];
extern int              ival;
extern double           rval;

extern TABLE            gsyms,
                        lsyms;
extern SYM              *lasthead;
extern struct slit      *strtab;
extern int              lc_static;
extern int              lc_auto;
extern struct snode     *bodyptr;       /* parse tree for function */
extern int              global_flag;
extern TABLE            defsyms;
extern int              save_mask;      /* register save mask */
extern int              optflag;	/* optimization ON or NOT */
extern char             infile[MAX_FILE],
                        listfile[MAX_FILE],
                        outfile[MAX_FILE],
                        copyfile[MAX_FILE],
                        incfile[MAX_FILE];
extern int		mainflag;
extern int              optimized;	/* this pass optimized or not */
extern int              listflag;       /* list flag ON  */
extern int              copyflag;       /* copy flag OFF */
extern char             inc_files[MAX_INCLUDE];	/* storage for include names  */
extern char             src_files[MAX_FILE];    /* storage for source redirect */
extern char             *incptr;        /* include pointer */
extern char             *srcptr;        /* source redirect pointer */
extern int		total_errors;	/* total # of errors */
extern char		inline[132];	/* input line */
extern int		numerrs;	/* number of errors */
extern int		errnos[MAX_ERROR];	/* error numbers */
extern char		*linstack[20];	/* stack for substitutions */
extern char		chstack[20];	/* place to save lastch */
extern int		lstackptr;	/* substitution stack pointer */
extern struct cse       *olist;         /* list of optimizable expressions */
extern TYP              *head;
extern TYP              *tail;
extern char             *declid;
extern TABLE            tagtable;
extern TYP              stdconst;
extern TYP              stdint;
extern TYP              stduint;
extern TYP              stdchar;
extern TYP              stdstring;
extern TYP              stdfunc;
extern int              breaklab;
extern int              contlab;
extern int              retlab;
extern int              glbsize;        /* size left in current global block */
extern int              mansize;	/* size left in peepgen list */
extern int              locsize;        /* size left in current local block */
extern int		asmsize;	/* size left in current assembly blk */
extern int              glbindx;        /* global index */
extern int              manindx;	/* peepgen index */
extern int              locindx;        /* local index */
extern int		asmindx;	/* assembly memory index */
extern struct blkmem    *locblk;        /* pointer to local memory block */
extern struct blkmem    *glbblk;        /* pointer to global memory block */
extern struct blkmem    *manblk;	/* pointer to peepgen memory block */
extern struct blkmem	*asmblk;	/* pointer to assembly memory block */
extern int              outcol;
extern struct ocode     *peep_head;	/* peepgen head */
extern struct ocode     *peep_tail;	/* peepgen tail */
extern FILE            *inclfile[MAX_FILE];
extern int             incldepth;
extern int             inclline[10];
extern int             ifdepth;		/* if depth */
extern int             ifarray[10];	/* if array */
extern char            *lptr;
extern int             next_data;	/* used for register processing */
extern int             max_data;
extern int             regflag;         /* register optimization flag */
extern int             windowflag;	/* format output for windows flag */
extern int             manlis;          /* which list am I managing (0=loc) */
extern int             quiet;           /* quiet flag */
extern int	       assembly;	/* processing assembly (1)or not (0) */
