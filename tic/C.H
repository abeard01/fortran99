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

/*      compiler header file    */

enum e_sym {
        id, cconst, iconst, uiconst,  lconst, sconst, rconst, 
        plus, minus,
        star, divide, lshift, rshift, modop, eq, neq, lt, leq, gt,
        geq, assign, asplus, asminus, astimes, asdivide, asmodop,
        aslshift, asrshift, asand, asor, autoinc, autodec, hook, compl,
        comma, colon, semicolon, uparrow, openbr, closebr, begin, end,
        openpa, closepa, pointsto, dot, lor, land, not, or, and, kw_int,
        kw_void, kw_char, kw_float, kw_double, kw_struct, kw_union,
        kw_long, kw_short, kw_unsigned, kw_auto, kw_extern,
        kw_register, kw_typedef, kw_static, kw_goto, kw_return,
        kw_sizeof, kw_break, kw_continue, kw_if, kw_else, kw_for,
        kw_do, kw_while, kw_switch, kw_case, kw_default, kw_enum,
        kw_eof, kw_include, kw_ifdef, kw_ifndef, kw_define, kw_endif,
        kw_asm, kw_endasm };

enum e_sc {
        sc_static, sc_auto, sc_global, sc_external, sc_type, sc_const,
        sc_member, sc_label, sc_ulabel };

enum e_bt {
        bt_char, bt_short, bt_long, bt_float, bt_double, bt_pointer,
        bt_unsigned, bt_struct, bt_union, bt_enum, bt_func, bt_ifunc,
        bt_void};

struct slit {
        struct slit     *next;
        int             label;
        char            *str;
        };

struct sym {
        struct sym *next;
        char  *name;
        enum e_sc storage_class;
        union   {
                short           i;
                unsigned        u;
                double          f;
                char            *s;
                }
                        value;

        struct typ {
                enum e_bt type;
                char val_flag;       /* is it a value type */
                int  size;
                struct stab {
                        struct sym      *head, *tail;
                        }       lst;
                struct typ      *btp;
                char            *sname;
                } *tp;
        };

#define SYM     struct sym
#define TYP     struct typ
#define TABLE   struct stab

#define MAX_STRLEN      120
#define MAX_STLP1       121

#define ERR_SYNTAX      0
#define ERR_ILLCHAR     1
#define ERR_FPCON       2
#define ERR_ILLTYPE     3
#define ERR_UNDEFINED   4
#define ERR_DUPSYM      5
#define ERR_PUNCT       6
#define ERR_IDEXPECT    7
#define ERR_NOINIT      8
#define ERR_INCOMPLETE  9
#define ERR_ILLINIT     10
#define ERR_INITSIZE    11
#define ERR_ILLCLASS    12
#define ERR_BLOCK       13
#define ERR_NOPOINTER   14
#define ERR_NOFUNC      15
#define ERR_NOMEMBER    16
#define ERR_LVALUE      17
#define ERR_DEREF       18
#define ERR_MISMATCH    19
#define ERR_EXPREXPECT  20
#define ERR_WHILEXPECT  21
#define ERR_NOCASE      22
#define ERR_DUPCASE     23
#define ERR_LABEL       24
#define ERR_PREPROC     25
#define ERR_INCLFILE    26
#define ERR_CANTOPEN    27
#define ERR_DEFINE      28
#define ERR_ENDIF	29
#define ERR_DEPTH	30
#define ERR_UNKNOWN     31

/*      alignment sizes         */

#define AL_CHAR         1
#define AL_SHORT        2
#define AL_LONG         2
#define AL_POINTER      2
#define AL_FLOAT        2
#define AL_DOUBLE       2
#define AL_STRUCT       2

#define TRUE            1
#define FALSE           0

#define MAX_INCLUDE     200		/* # of bytes for other include names */
#define MAX_FILE        40		/* maximum size of a file name */
#define MAX_ERROR       2               /* maximum # of errors per line */

#define LOCALIST        0		/* local list */
#define PEEPLIST	1		/* peep list */
#define ASMLIST         2               /* assembly source list */
