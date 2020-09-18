/*
 *      TMS9900 C Compiler
 *
 *      code structure definitions.
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

enum address_mode {
        am_const, am_label, am_string, am_temp, am_auto,
        am_defcon, am_deflab, am_defstr, am_deftemp, am_defauto,
        am_none };

enum instruction {
        i_move, i_add, i_sub, i_mul, i_div, i_mod, i_and, i_or,
        i_xor, i_shl, i_shr, i_jmp, i_jeq, i_jne, i_jlt, i_jle,
        i_jgt, i_jge, i_call, i_enter, i_ret, i_table, i_label };

union aval {
        int             i;
        char            *s;
        };

struct inst {
        struct inst     *fwd, *back;
        int             opcode, size;
        char            sm0, sm1, dm;
        union aval      sv0, sv1, dv;
        };
