/*   a99.h                                                           */
/*   HEADER file for three part split of a99.                        */
/*                                                                   */
/*  note:  MAXLINES changed for 8 lines/inch paper (76 lines)        */

#include "a99proto.h"               /* include prototypes */

#define VERSION "TI 99/4A and 9640 Cross-Assembler Version 2.12 (29dec90)"
#define OPENRD "r"                  /* open for read access */
#define OPENWT "w"                  /* open for write access */
#define OPENWB "wb"                 /* open for binary write access */
#define TRUE 1                      /* something is true def */
#define FALSE 0                     /* something is false def */
#define MAXERRC 400                 /* maximum size of error messages */
#define MAXSYM 12000                /* maximum symbol table space */
#define MAXVAL 2000                 /* maximum # of symbols */
#define MAXTOK 80                   /* size of maximum token length */
#define MAXMACROS 80                /* maximum # of macros */
#define MAXMACTXT MAXMACROS*250     /* 250 chars/macro */
#define MAXPARS 180                 /* maximum space for parameters */
#define MAXOPC 101                  /* maximum number of opcodes */
#define MNEVER 0                    /* when to list macros, never */
#define MONCE 1                     /* when to list macros, once */
#define MALWAYS 2                   /* when to list macros, always */
#define MOBJECT 3                   /* when to list macros, object only */
#define MAXTITL 50                  /* Maximum chars in TITL statement */
#define MAXLINE 76                  /* Maximum lines per page */
#define NEWOLIN 59                  /* Column to flush the object line */
#define NEWCLIN 78		    /* Column to flush the compressed obj */
#define FF 0x0C00                   /* Form Feed String */
#define QUOTE 39                    /* Single quote (') */
#define ABSTYP 1                    /* Absolute value type */
#define REFTYP 2                    /* Reference symbol type */
#define RELTYP 3                    /* Relative symbol type */

#ifdef MAIN

/* globals for all routines */

int alineno,                /* current line # in file */
    atend,                  /* TRUE when END statement found */
    chksum,                 /* record check sum processing */
    compobj,                /* compressed object (1) or not (0) */
    ensym,                  /* index of start of next new symbol */
    enval,                  /* enval is index of next value */
    errs,                   /* number of errors total */
    ex9900,                 /* 9900 opcode extentions */
    ex99000,                /* 99000 opcode extentions */
    genobject,              /* generate object (TRUE) or not (FALSE) */
    gtptr,                  /* pointer into line, gtoken pointer */
    iffflag,                /* Currently IF inhibited or NOT */
    incflag,                /* Currently INCLUDING or NOT */
    inhflag,                /* Currently Inhibiting OBJECT or not */
    lineno,                 /* current line # on page */
    listmacros,             /* flag to control listing of macros */
    liston,                 /* listing on (TRUE) or not (FALSE) */
    listst,                 /* LIST statement encountered (TRUE) */
    listtext,               /* flag to control listing of text */
    loc,                    /* location counter */
    macfirst,               /* true first time expanding a macro */
    obflg,                  /* first object code generated */
    objbyt,                 /* actual byte being held */
    objhel,                 /* byte being held due to odd value */
    objloc,                 /* object byte held location */
    objon,                  /* object file on (TRUE) or not (FALSE) */
    objptr,                 /* output pointer to object line */
    objseq,                 /* object code sequence number */
    oboldl,                 /* last location output */
    p2eptr,                 /* index to p2estr */
    p2errout,               /* flag if error string needs to be output */
    pageno,                 /* current page # */
    passnr,                 /* pass number we are on: 1 or 2 */
    ploc,                   /* psuedo location counter, for REF's */
    registers,              /* registers to be ENTERED or not */
    relflag,                /* generating relative object or not */
    extended,               /* Extended Assembler or not */
    xopcode,                /* X opcode for segmented loader */
    symval[MAXVAL+2],       /* values of regular symbols */
    symtyp[MAXVAL+2],       /* symbol table type of symbol */
    symind[MAXVAL+2];       /* index into symt */


char buff[80],              /* working buffer */
    copyf[80],              /* copy file name */
    idt[9],                 /* program ident, 8 chars max */
    inputf[80],             /* name of input file */
    labelname[MAXTOK],      /* holds current label name */
    line[180],              /* input line */
    listf[80],              /* name of listing file */
    mline[180],             /* macro saved input line */
    objf[80],               /* name of object file */
    objline[81],            /* object code line */
    p2estr[MAXERRC+4],      /* holds cumulative error string */
    symt[MAXSYM+10],        /* regular symbol table */
    titl[MAXTITL+1],        /* comment string holds title line */
    tok[MAXTOK+1];          /* holds current token various places - temp */
                            /* 1-14-88 increased tok[] by 1 for null???*/

#else


/* globals for all routines */

extern int alineno,         /* current line # in file */
    atend,                  /* TRUE when END statement found */
    chksum,                 /* record check sum processing */
    compobj,                /* compressed object (1) or not (0) */
    ensym,                  /* index of start of next new symbol */
    enval,                  /* enval is index of next value */
    errs,                   /* number of errors total */
    ex9900,                 /* 9900 opcode extentions */
    ex99000,                /* 99000 opcode extentions */
    genobject,              /* generate object (TRUE) or not (FALSE) */
    gtptr,                  /* pointer into line, gtoken pointer */
    iffflag,                /* Currently IF inhibited or NOT */
    incflag,                /* Currently INCLUDING or NOT */
    inhflag,                /* Currently Inhibiting OBJECT or not */
    lineno,                 /* current line # on page */
    listmacros,             /* flag to control listing of macros */
    liston,                 /* listing on (TRUE) or not (FALSE) */
    listst,                 /* LIST statement encountered (TRUE) */
    listtext,               /* flag to control listing of text */
    loc,                    /* location counter */
    macfirst,               /* true first time expanding a macro */
    obflg,                  /* first object code generated */
    objbyt,                 /* actual byte being held */
    objhel,                 /* byte being held due to odd value */
    objloc,                 /* object byte held location */
    objon,                  /* object file on (TRUE) or not (FALSE) */
    objptr,                 /* output pointer to object line */
    objseq,                 /* object code sequence number */
    oboldl,                 /* last location output */
    p2eptr,                 /* index to p2estr */
    p2errout,               /* flag if error string needs to be output */
    pageno,                 /* current page # */
    passnr,                 /* pass number we are on: 1 or 2 */
    ploc,                   /* psuedo location counter, for REF's */
    registers,              /* generate register definitions or not */
    relflag,                /* generating relative object code or not */
    extended,               /* extended assembler compatibility */
    xopcode,                /* X opcode for segmented loader */
    symval[MAXVAL+2],       /* values of regular symbols */
    symtyp[MAXVAL+2],       /* symbol table type of symbol */
    symind[MAXVAL+2];       /* index into symt */


extern char buff[80],       /* working buffer */
    copyf[80],              /* copy file name */
    idt[9],                 /* program ident, 8 chars max */
    inputf[80],             /* name of input file */
    labelname[MAXTOK],      /* holds current label name */
    line[180],              /* input line */
    listf[80],              /* name of listing file */
    mline[180],             /* macro saved input line */
    objf[80],               /* name of object file */
    objline[81],            /* object code line */
    p2estr[MAXERRC+4],      /* holds cumulative error string */
    symt[MAXSYM+10],        /* regular symbol table */
    titl[MAXTITL+1],        /* comment string holds title line */
    tok[MAXTOK+1];          /* holds current token various places - temp */
                            /* 1-14-88 increased tok[] by 1 for null???*/

#endif


/*   MACRO facility data structures */

#ifdef MAIN

struct macro{
        char *mname;        /* name of macro */
        char *mbody;        /* body of macro */
        int used;           /* true when macro has been used */
};

struct macro macros[MAXMACROS]; /* macro table */

char mactxt[MAXMACTXT],     /* names and bodies of macros */
    *mpars[10],             /* up to 9 paramteres */
    msymgen[2],             /* symbol generator for locals */
    partab[MAXPARS],        /* table to hold parameters */
    *nxtexpand;             /* pointer to next char to expand */

int nxtmac,                 /* index to next macro to use */
    nmactxt,                /* index into next mactxt */
    in_macro,               /* flag if expanding macro */
    expanding,              /* TRUE if expanding a macro */
    mexpand,                /* which macro we are expanding */
    parused[10];

FILE *chin, *chobj, *chlst, *fopen(), *chinc;   /* file pointers */

#else
extern struct macro{
        char *mname;        /* name of macro */
        char *mbody;        /* body of macro */
        int used;           /* true when macro has been used */
};

extern struct macro macros[MAXMACROS];  /* macro table */

extern char mactxt[MAXMACTXT],      /* names and bodies of macros */
    *mpars[10],                     /* up to 9 paramteres */
    msymgen[2],                     /* symbol generator for locals */
    partab[MAXPARS],                /* table to hold parameters */
    *nxtexpand;                     /* pointer to next char to expand */

extern int nxtmac,                  /* index to next macro to use */
    nmactxt,                        /* index into next mactxt */
    in_macro,                       /* flag if expanding macro */
    expanding,                      /* TRUE if expanding a macro */
    mexpand,                        /* which macro we are expanding */
    parused[10];

extern FILE *chin, *chobj, *chlst, *fopen(), *chinc;/* file pointers */

#endif


