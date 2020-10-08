
/* a99pass2.c - This module contains the second pass routines, starting
                with pass2          */

/* ===============================>>> pass2 <<<==============================*/

#include <stdio.h>
#include "a99.h"

pass2()

{
char f1, f2, typ, delim, elvs[6];
int v1, v2, ov1, ov2, i, sindex, t2, kind, kind1, kind2;
long val = 0;               /*1-17-89 int to long */

/* Initialize various things */

    atend = FALSE;          /* Not at end of program */
    p2errout = FALSE;
    p2eptr = 0;
    lineno = MAXLINE;       /* line number on page, force header */
    alineno = 0;            /* actual line number in file */
    pageno = 0;             /* page number */
    objseq = 0;             /* object line sequence number */
    objptr = 0;             /* object line output pointer */
    objhel = FALSE;         /* object byte held waiting for next flag */
    genobject = TRUE;       /* generate object or not */
    listst  = TRUE;         /* TRUE until UNL statement encountered */
    incflag = FALSE;        /* INCLUDE file (NO) */
    iffflag = FALSE;        /* Processing IF or NOT */
    inhflag = FALSE;        /* Inhibit source generation or not */

/* initialize title to blank */
    for (i=0; i<MAXTITL; i++){
        titl[i] = ' ';
    }
    titl[MAXTITL] = 0;
    msymgen[0] = msymgen[1] = 'a';  /* symbol generator for macro locals */

/* Set identifier based on what was read in PASS1 */
    if (relflag){                   /* if generating relative code */
        if ( compobj ){
            objline[objptr++] = 01;
            objline[objptr++] = loc / 256 & 0xff;
            objline[objptr++] = loc       & 0xff;
        }else{
            objline[objptr++] = '0';
            wtohs (loc, elvs);          /* convert ending loc to hex */
            for (i=0; i<4; ++i){
                objline[objptr++] = elvs[i];
	    }
        }
    }else{                          /* generating absolute code */
        if ( compobj ) {
            objline[objptr++] = 01;
            objline[objptr++] = 0;
            objline[objptr++] = 0;
	}else{
            objline[objptr++] = '0';
            objline[objptr++] = '0';        /* # of bytes of relocatable code */
            objline[objptr++] = '0';
            objline[objptr++] = '0';
            objline[objptr++] = '0';
	}
    }
    relflag = TRUE;                 /* assume generating relative */
    loc = 0x0000;                   /* location reset */
/* user identifier (from pass 1) */
    for (i=0; i<8; i++){
        objline[objptr++] = idt[i];
    }

P2L0:                               /* to here for each new line */
    labelname[0] = 0;

P2L1:
    if (atend){
        return TRUE;
    }
    if (!gtok(tok, &delim)){
        return TRUE;
    }
    while (delim == ':'){           /* label? */
        strcpy(labelname, tok);
        upper(tok);                 /* upper so can check it */
        if (*tok < 'A' || *tok > 'Z'){
            if (*tok == '?'){
                if (!expanding){
                    p2errt("Labels with ? only allowed in macros: ", labelname);
                }
            }else if (*tok != '$' && *tok != '.' && *tok != '_'){
                p2errt("Label must begin with a-z, $, _ or .: ", labelname);
            }
        }
        gtok(tok, &delim);          /* might be equ */
        if (eqlstr(tok, "EQU") || eqlstr(tok, "MACRO")){
            typ = 'P';
            goto P2EQU;
        }
        if (!(sindex = lookup(labelname, &val))){ /* 1-11-89 removed (long*)*/
            p2errt("Phase error - symbol undefined 2nd Pass: ", labelname);
        }
        if (val != loc){
            p2errt("Multiple definition of symbol: ", labelname);
        }
    }

/* have op code now */
    gopcod(tok, &typ, &val);        /* fetch op code typ, value */

P2EQU:
    if (typ == 'P'){                /* pseudo op */
        psop(tok, delim);
        goto P2NXT;
    }

/* check for different types of opcodes now */
    if ( objhel ){
      emitbyte(loc,0);
    }
    loc = ++loc & 0xfffe;	    /* force an EVEN directive */
    ploc = loc;
    if (typ == 'A'){
      emit(loc, val, line);         /* emit as is opcode */
    }else{
    if (delim != ' '){
        p2err("Missing operand(s) or bad syntax");
        gtptr = 200;
        emit(loc, 0, line);
        goto P2EOL;
    }

    if (typ == 'C'){                /* CRU type, needs displacement */
        gtok(tok, &delim);
        t2 = eval(tok, &v1);
        if (v1 > 127 || v1 < -128){
            p2err("-128 <= CRU displacement <= 127 required");
        }
        if (t2 != ABSTYP){
            p2err("CRU displacement must be absolute");
        }
        v1 &= 0xff;
        val |= v1;
        emit(loc, val, line);

    }else if (typ == 'D'){          /* double general operand */
        gopnd(&v1, &f1, &ov1, &delim, &kind1);
        if (delim != ','){
            p2err("Missing second operand");
            gtptr = 200;
            emit(loc, 0, line);
            if (f1 == 'Y'){         /* must agree with pass 1 */
                emval(0);
            }
            goto P2EOL;
        }

        gopnd(&v2, &f2, &ov2, &delim, &kind2);
        v2 = (v2 << 6) & 0xffc0;        /* shift left 6 */
        val = val | v2 | v1;
        emit(loc, val, line);

/* if flag 1 (f1) or flag 2 (f2) are set, emit the appropriate obejct
   for the desired symbol mode (absolute or relative) */

        if (f1 == 'Y'){		        /* first opcode needed */
	    if (kind1 == ABSTYP){
	        emval(ov1);		/* emit absolute object */
	    }else if (kind1 == RELTYP){
	        emvalr(ov1);		/* emit relative object */
	    }else if (relflag & (ov1 != 0)){
	        emvalref(ov1);		/* emit relative REF */
	    }else{
	        emval(ov1);		/* emit absolute REF */
	    }
	}
	if (f2 == 'Y'){		        /* second opcode needed */
	    if (kind2 == ABSTYP){
	        emval(ov2);		/* emit absolute object */
	    }else if (kind2 == RELTYP){
	        emvalr(ov2);		/* emit relative object */
	    }else if (relflag & (ov2 != 0)){
	        emvalref(ov2);		/* emit relative REF */
	    }else{
	        emval(ov2);		/* emit absolute REF */
	    }
	}
    }else if (typ == 'I'){      /* immediate, needs register and value */
        greg(&v1, &delim);
        if (delim != ','){
            p2err("Missing immediate operand");
            gtptr = 200;
            emit(loc, 0, line);
            emval(0);           /* space for the immediate anyway */
            goto P2EOL;
        }
        val |= v1;
        emit(loc, val, line);
        gtok(tok, &delim);
        ploc += 2;
        t2 = eval(tok, &v2);
        loc += 2;
        if (t2 == ABSTYP){
            emit(loc, v2, " ");
        }else if (t2 == RELTYP){
            emitr(loc, v2, " ");
        }else if (relflag & (v2 != 0)){
            emitlref(loc, v2, " ");
        }else{
            emit (loc, v2, " ");
	}
    }else if (typ == 'J'){      /* jump displacement */
        gtok(tok, &delim);
        if ((t2 = eval(tok, &v1)) == REFTYP){
            p2err("Relative jump cannot be REF");
        }
        v2 = (v1 - loc - 2) / 2;
        if (v2 > 127 || v2 < -128){
              p2err("Relative jump out of range");
        }
        v2 &= 0xff;
        val |= v2;
        emit(loc, val, line);
    }else if (typ == 'N'){      /* nibble type, needs general opnd, 0-15 val */
        gopnd(&v2, &f2, &ov2, &delim, &kind);
        if (delim != ','){
            p2err("Missing operand");
            gtptr = 200;
            emit(loc, 0, line);
            goto P2EOL;
        }
        val |= v2;
        g015(&v1, &delim);
        v1 = (v1 << 6) & 0xffc0;
        val |= v1;
        emit(loc, val, line);
        if (f2 == 'Y'){
            if (kind == ABSTYP){
                emval(ov2);
            }else if (kind == RELTYP){
                emvalr(ov2);
            }else if (relflag & (ov2 != 0)){
                emvalref(ov2);
            }else{
            emval(ov2);
            }
        }
    }else if(typ == 'O'){       /* one general operand */
        gopnd(&v1, &f1, &ov1, &delim, &kind);
        val |= v1;
        emit(loc, val, line);
        if (f1 == 'Y'){         /* if first opcode needed */
            if (kind == ABSTYP){
                emval(ov1);
            }else if (kind == RELTYP){
                emvalr(ov1);
            }else if (relflag & (ov1 != 0)){
                emvalref(ov1);
            }else{
                emval(ov1);
            }
	}
    }else if (typ == 'R'){      /* internal register immediate */
        emit(loc, val, line);
        gtok(tok, &delim);
        t2 = eval(tok, &v1);
        loc += 2;
        if (t2 == ABSTYP){
            emit(loc, v1, " ");
        }else if (t2 == RELTYP){
            emitr(loc, v1, " ");
        }else if (relflag & (v1 != 0)){
            emitlref(loc, v1, " ");
        }else{
            emit (loc, v1, " ");
        }
    }else if (typ == 'S'){      /* shift op, needs count, register */
        greg(&v2, &delim);
        if (delim != ','){
            p2err("Missing count operand");
            gtptr = 200;
            emit(loc, 0, line);
            goto P2EOL;
        }
        g015(&v1, &delim);
        v1 = (v1 << 4) & 0xfff0;
        val = val | v1 | v2;
        emit(loc, val, line);
    }else if (typ == 'W'){      /* work space, general op */
        gopnd(&v1, &f1, &ov1, &delim, &kind);
        if (delim != ','){
            p2err("Missing operand");
            gtptr = 200;
            emit(loc, 0, line);
            if (f1 == 'Y'){
                emval(0);       /* needs space for operand */
            }
            goto P2EOL;
        }
        greg(&v2, &delim);
        v2 = (v2 << 6) & 0xffc0;
        val = val | v2 | v1;
        emit(loc, val, line);
        if (f1 == 'Y'){         /* if first opcode needed */
            if (kind == ABSTYP){
                emval(ov1);
            }else if (kind == RELTYP){
                emvalr(ov1);
            }else if (relflag & (ov1 != 0)){
                emvalref(ov1);
            }else{
                emval(ov1);
            }
        }
    }else if (typ == 'Z'){      /* internal register */
        greg(&v1, &delim);
        val |= v1;
        emit(loc, val, line);

/* New TMS99000 opcode types */
    }else if(typ == 'B'){       /* bit number, one general operand  */
        emit(loc, val, line);
        g015(&val, &delim);
        val = (val << 6) & 0x03c0;
        if (delim != ','){
            p2err("Missing operand");
            gtptr = 200;
            emit(loc, 0, line);
            emval(0);           /* needs space for operand */
            goto P2EOL;
        }
        gopnd(&v1, &f1, &ov1, &delim, &kind);
        val |= v1;
        emval(val);
        if (f1 == 'Y'){         /* if first opcode needed */
            if (kind == ABSTYP){
                emval(ov1);
            }else if (kind == RELTYP){
                emvalr(ov1);
            }else if (relflag & (ov1 != 0)){
                emvalref(ov1);
            }else{
                emval(ov1);
            }
        }
    }else if (typ == 'G'){      /* double precision arithmetic */
        emit(loc, val, line);     /* initial opcode value */
        val = 0x4000;           /* second opcode value */
        gopnd(&v1, &f1, &ov1, &delim, &kind1);
        if (delim != ','){
            p2err("Missing second operand");
            gtptr = 200;
            emit(loc, 0, line);
            if (f1 == 'Y'){     /* must agree with pass 1 */
                emval(0);
            }
            goto P2EOL;
        }
        gopnd(&v2, &f2, &ov2, &delim, &kind2);
        v2 = (v2 << 6) & 0xffc0;    /* shift left 6 */
        val = val | v2 | v1;
        emval(val);

/* if flag 1 (f1) or flag 2 (f2) are set, emit the appropriate obejct
   for the desired symbol mode (absolute or relative) */

        if (f1 == 'Y'){             /* first opcode needed */
            if (kind1 == ABSTYP){
                emval(ov1);         /* emit absolute object */
            }else if (kind1 == RELTYP){
                emvalr(ov1);        /* emit relative object */
            }else if (relflag & (ov1 != 0)){
                emvalref(ov1);        /* emit relative REF */
            }else{
                emval(ov1);             /* emit absolute REF */
	    }
	}
        if (f2 == 'Y'){             /* second opcode needed */
            if (kind2 == ABSTYP){
                emval(ov2);         /* emit absolute object */
            }else if (kind2 == RELTYP){
                emvalr(ov2);        /* emit relative object */
            }else if (relflag & (ov2 != 0)){
                emvalref(ov2);      /* emit relative REF */
            }else{
                emval(ov2);         /* emit absolute REF */
            }
	}
        }else if (typ == 'F'){      /* dual operand floating point */
            emit(loc, val, line);     /* initial opcode value */
            val = 0x0000;           /* second opcode value */
            gopnd(&v1, &f1, &ov1, &delim, &kind1);
            if (delim != ','){
                p2err("Missing second operand");
                gtptr = 200;
                emit(loc, 0, line);
                if (f1 == 'Y'){     /* must agree with pass 1 */
                    emval(0);
                }
                goto P2EOL;
            }
            gopnd(&v2, &f2, &ov2, &delim, &kind2);
            v2 = (v2 << 6) & 0xffc0;    /* shift left 6 */
            val = val | v2 | v1;
            emval(val);

/* if flag 1 (f1) or flag 2 (f2) are set, emit the appropriate obejct
   for the desired symbol mode (absolute or relative) */

            if (f1 == 'Y'){             /* first opcode needed */
                if (kind1 == ABSTYP){
                    emval(ov1);         /* emit absolute object */
                }else if (kind1 == RELTYP){
                    emvalr(ov1);        /* emit relative object */
                }else if (relflag & (ov1 != 0)){
                    emvalref(ov1);      /* emit relative REF */
                }else{
                    emval(ov1);         /* emit absolute REF */
                }
	    }
            if (f2 == 'Y'){             /* second opcode needed */
                if (kind2 == ABSTYP){
                    emval(ov2);         /* emit absolute object */
                }else if (kind2 == RELTYP){
                    emvalr(ov2);        /* emit relative object */
                }else if (relflag & (ov2 != 0)){
                    emvalref(ov2);        /* emit relative REF */
                }else{
                    emval(ov2);         /* emit absolute REF */
                }
	    }
        }else if(typ == 'M'){           /* double precision shift instruction */
            emit(loc, val, line);       /* emit 16-bit opcode */
            val = 0x4000;               /* initial opcode 2 */
            gopnd(&v1, &f1, &ov1, &delim, &kind);
            val |= v1;
            if (delim != ','){
                p2err("Missing operand");
                gtptr = 200;
                emval(0);               /* needs space for operand */
                goto P2EOL;
            }
            g015(&v1, &delim);
            val = (v1 << 6) + val;
            emval(val);
            if (f1 == 'Y'){             /* if first opcode needed */
                if (kind == ABSTYP){
                    emval(ov1);
                }else if (kind == RELTYP){
                    emvalr(ov1);
                }else if (relflag & (ov1 != 0)){
                    emvalref(ov1);
                }else{
                    emval(ov1);
                }
            }
	}else{                          /* should never get here */
        printf("Fatal Error: Invalid internal instruction type\n");
        exit(999);
        }
    }
/* end of not A opcodes */

P2EOL:
    loc +=2;                            /* bump loc */
    gtptr = 200;                        /* eat rest of the line */

P2NXT:
    if (p2errout){                      /* errors output */
        showerrors();
        p2eptr = 0;
        p2errout = FALSE;
    }
    goto P2L0;
}

/* ===============================>>> p2err <<<==============================*/
p2err(err)

char *err;
{
    if (passnr != 2){
        return TRUE;
    }
    ++errs;
    if (p2eptr > 0){
        p2estr[p2eptr++] = 9;
    }
    for (; *err && p2eptr < MAXERRC; ++err){
        p2estr[p2eptr++] = *err;
    }
    p2estr[p2eptr++] = '\n';
    p2estr[p2eptr] = 0;         /* terminate but don't bump */
    p2errout = TRUE;
}

/* ===============================>>> p2errt <<<==============================*/
p2errt(err, what)

char *err, *what;
{
    if (passnr != 2){
        return TRUE;
    }
    ++errs;
    if (p2eptr > 0){
        p2estr[p2eptr++] = 9;
    }
    for (; *err && p2eptr < MAXERRC; ++err){
        p2estr[p2eptr++] = *err;
    }
    for (; *what && p2eptr < MAXERRC; ++what){
        p2estr[p2eptr++] = *what;
    }
    p2estr[p2eptr++] = '\n';
    p2estr[p2eptr] = 0;         /* terminate but don't bump */
    p2errout = TRUE;
}

/* ===============================>>> set2 <<<==============================*/
void set2(what, val)

int *what, val;
{
    if (passnr == 2){
        *what = val;
    }
}

/* ===============================>>> showerrors <<<=========================*/
showerrors()

{
    if (passnr != 2){
        return TRUE;
    }
    printf("---------------------------------------------------------------\n");
    printf("%4d %s\n", alineno, line);
    printf("**** Error(s): %s\n", p2estr);
    lineno++;
    if (liston){
      fprintf(chlst, "**** Error(s): %s\n", p2estr);
    }
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                                                           */
/*     Code for macros follows                                               */
/*                                                                           */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* ==========================>>> addtxt <<<=========================== */
addtxt(txt, terminator)

char *txt, terminator;
{
    /*  add txt to mactxt buffer.  p2err if out of space, return null.
        If successfully added, return char * to start of string */
    if (passnr == 2){
        return TRUE;
    }
    for (; *txt; ++txt){
        if (nmactxt >= MAXMACTXT){
            fprintf(stderr, "\nFATAL error: Out of macro table space on %s\n",
                    labelname);
            exit(0);
        }
        mactxt[nmactxt++] = *txt;
    }
    mactxt[nmactxt++] = terminator;
}

/* ==========================>>> getmline <<<============================== */
getmline(ln)

char *ln;
{
    /*  getmline does the actual macro expansion.  It retrieves the next
        line from the currently selected macro, expands all ?X temp labels
        and #n parameter references.  When macro body runs out, getmline
        should free all used parameters, and set expanding to false  */

    char curchr, *cp;
    int i;

    if (*nxtexpand == 0){
        for (i = 1; i < 10; ++i){
            if (mpars[i] != NULL && !parused[i]){
                p2errt("Parameter defined but not used: ", mpars[i]);
            }
     }
        /* because of way newlin and getmline interact, need  */
        /* to check for errors at this point explicitly       */
        if (p2errout && passnr == 2){
            showerrors();
            p2eptr = 0;
            p2errout = FALSE;
        }
        expanding = FALSE;              /* done with expansion */
        macfirst = FALSE;               /* no longer first time */
        return FALSE;
    }
    while (*nxtexpand != '\n'){
        curchr = *nxtexpand;            /* get the current character */
        switch (curchr){
            case '?':                           /* label */
                    *ln++ = '?';
                    ++nxtexpand;                /* point to next char */
                    if (clower(*nxtexpand) < 'a' || clower(*nxtexpand) > 'z'){
                        p2err("Invalid ?x label in macro expansion: 'a' <= x <= 'z'");
                    }
                    *ln++ = *nxtexpand++;       /* copy next char no matter what */
                    if (++msymgen[1] > 'z'){    /* bump symbol generator */
                        msymgen[1] = 'a';
                        if (++msymgen[0] > 'z'){
                            msymgen[0] = '0';
                            p2err("Macro symbol generator overflow - too many ? labels");
                        }
                    }
                    *ln++ = msymgen[0];
                    *ln++ = msymgen[1];         /* add local label */
                    break;

            case '%':                           /* parameter */

                    ++nxtexpand;                /* eat the % */
                    curchr = *nxtexpand++;      /* parameter number next */
                    if (curchr < '1' || curchr > '9'){
                        p2err("Invalid parameter in macro body, must be 1..9");
                        curchr = '1';           /* assume 1 */
                    }
                    i = curchr - '0';           /* convert to offset */
                    if (mpars[i] == NULL){
                        p2err("Parameter used in macro body not provided in call");
                        *ln++ = '0';
                    }else{
                    for (cp = mpars[i]; *cp; ++cp){
                        *ln++ = *cp;
                    }
                    parused[i] = TRUE;
                    }
                    break;

            case '[':                           /* quoted string */
                    ++nxtexpand;                /* eat the [ */
                    while (*nxtexpand != ']' && *nxtexpand != '\n'){
                        *ln++ = *nxtexpand++;
                    }
                    if (*nxtexpand != ']'){
                        p2err("Missing closing ] in macro body expansion");
                    }else{
                    ++nxtexpand;                /* skip ] */
                    }
                    break;

            default:
                *ln++ = *nxtexpand++;
                break;
        }
    }
    *ln++ = 0;
    if (*nxtexpand == '\n'){
        ++nxtexpand;            /* skip \n */
    }
    return TRUE;

}

/* ==========================>>> expandmacro <<<=========================== */
expandmacro(mcode, mdelim)

char *mcode, mdelim;
{
    /*  expandmacro will echo line like a comment, then get all parameters.
        It sets expanding to true so that getmline can carry out expansion. */

    int i, j, nxtpar;
    char d;

    if (listmacros == MOBJECT){         /* save line for listing */
        strcpy (mline, line);
    }else{
        emmaccall(loc, line);           /* echo this line */
    }
    for (i = 0; i < nxtmac; ++i){       /* search for macro */
        if (eqlstr(mcode, macros[i].mname)){
            goto MFOUND;
        }
    }
    p2errt("Undefined opcode or macro: ", mcode);
    gtptr = 200;
    if (passnr != 1){
        emtobj(loc, 0, '?');            /* emit one word for undefined */
    }
    return TRUE;

MFOUND:
    if (expanding){
        p2errt("Nested calls to macros not allowed: ", mcode);
        gtptr = 200;
        return TRUE;
    }
    mexpand = i;                        /* which macro we are expanding */
    nxtexpand = macros[mexpand].mbody;  /* point at start of body */
    if (passnr == 2){
        if (!macros[mexpand].used){     /* not used before */
        macfirst = TRUE;                /* first time we are expanding */
     }
    macros[mexpand].used = TRUE;        /* used now */
    }
    for (i = 0; i < 10; ++i){           /* null out parameters */
        mpars[i] = NULL;
        parused[i] = FALSE;
    }
    nxtpar = 0;

/* following code changed to allow blank as delimeter on macro line */
    for (j =  1, d = mdelim; d != 0 && j < 10; ++j){
        gtok(tok, &d);                   /* get parameter */
        mpars[j] = &partab[nxtpar];
        for (i = 0; tok[i]; ++i){
          partab[nxtpar++] = tok[i];    /* copy actual to parameter table */
        }
        partab[nxtpar++] = 0;
        if (nxtpar >= MAXPARS){
            printf("Fatal Error: Out of space for parameters to macro\n");
            exit(999);
        }
        if (d == ' ') d=0;              /* force skip of blanks */
    }
    if (j >= 10 && d != 0){             /* bug fixed 3/30/85, added && d!=0 */
        p2err("Too many parameters in macro call");
    }
    expanding = TRUE;                   /* and we are expanding */
    gtptr = 200;
}

/* ==========================>>> emmaccall <<<=========================== */
emmaccall(emloc, emline)

int emloc;
char *emline;
{
    char emstr[6], strout[5];

    if (passnr != 2){
       return TRUE;
    }
    chkpage();
    sprintf (strout, "%4d", alineno);
    emtsr(strout);
    emtsr(" ");
    wtohs(emloc, emstr);                 /* convert to hex string */
    emtsr(emstr);
    emtsr("   ----     ");
    emtsr(emline);
    emtsr("\n");
}

/* ==========================>>> savemacro <<<============================= */
savemacro()

{
    /*  save macro save a macro body - it will just echo each macro
        body line like a comment, then use gtok to squish macro body
        down as far as possible. Comments are striped out of macro body.
        savemacro controls all input until ENDM directive is found.  */

    char mtok[80], md;                           /* token, delim used by macro */
    char prevd;                                 /* previous delimiter */
    char *ch;

    if (nxtmac >= MAXMACROS){
        fprintf(stderr, "\nFATAL error: Out of macro entry space on %s\n", labelname);
        exit(999);
    }
    if (expanding){
        p2errt("Nested MACRO definitions not allowed: ", labelname);
        gtptr = 200;
        return TRUE;
    }
    if (labelname[0] == 0){
        p2err("Missing macro name");
        strcpy(labelname, "MACROXYYZ");
    }
    if (passnr == 1){
        macros[nxtmac].mname = &mactxt[nmactxt];    /* name start */
        for (ch = labelname; *ch; ++ch){
            *ch = cupper(*ch);                      /* force to upper case */
        }
        addtxt(labelname, 0);
        macros[nxtmac].used = FALSE;                /* not used yet */
        macros[nxtmac].mbody = &mactxt[nmactxt];    /* where we started */
    }else{
        outcom(line);
    }
    gtptr = 200;                        /* eat rest of line with MACRO */
    prevd = '\n';
    for (;;){                           /* read body of macro */
        if (atend){
            p2errt("Missing ENDM for macro ", macros[nxtmac].mname);
            break;
        }
        gtok(mtok, &md);                /* read a token */
        if (eqlstr(mtok, "ENDM")){      /* end? */
            if (passnr != 1 )
                outcom(line);		/* display line */
            if (prevd != '\n'){
                addtxt("\n", 0);        /* terminate macro body with \n */
            }else{
                addtxt("", 0);          /* terminate macro body */
            }
            break;                      /* leave loop */
        }
        if (md == 0){
	    if ( passnr != 1 ) 
              outcom(line);             /* echo in listing file */
            md = '\n';                  /* change 0 to \n */
        }
        if (md == ' '){
            md = '\t';                  /* spaces to tabs to make nice */
        }
        if (prevd == '\n' && md != ':'){
            addtxt("", '\t');           /* leading space for no label */
        }
        addtxt(mtok, md);
        if (md == ':'){
            addtxt("", '\t');
        }
        prevd = md;                     /* save delimiter */
    }
    if (passnr == 1){
        ++nxtmac;                       /* point to next macro */
    }
    gtptr = 200;                        /* eat rest of input line */
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                                                          */
/*     Misc. utility routines follow                                        */
/*                                                                          */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/*=============================>>> CLOWER  <<<================================*/
clower(ch)

char ch;
{
    return ((ch >='A' && ch<='Z') ? ch + ' ' : ch);
}

/*=============================>>> CUPPER  <<<================================*/
cupper(ch)

char ch;
{
    return ((ch >= 'a' && ch <= 'z') ? ch - ' ' : ch);
}

/* ===============================>>> dstow <<<==============================*/
dstow(dstr, dsv)

char *dstr;
long *dsv;
{
    /* decimal string to word */

    int ds1, ds2, ds3;

    ds2 = 0;
    for (ds1 = 0; dstr[ds1] != 0; ++ds1){
        ds3 = dstr[ds1];
        if (ds3 >= '0' && ds3 <= '9'){
            ds3 -= '0';
        }else{
        *dsv = 0;
        return FALSE;
        }
        ds2 = ds2*10+ds3;
    }
    *dsv = ds2;
    if (ds2 > 32768 || ds2 < -32768){
        return FALSE;
    }else{
        return TRUE;
    }
}

/* ===============================>>> eqlstr <<<==============================*/
eqlstr(s1, s2)

char *s1, *s2;
{
    for (; cupper(*s1) == cupper(*s2) && *s1; ++s1, ++s2)
        {;}
    if (*s1 == *s2){
        return TRUE;
    }
    return FALSE;
}

/* ===============================>>> hstow <<<==============================*/
hstow(hstr, hsv)

char *hstr;
long *hsv;
{
    /* hex string to word */

    int hs1, hs2, hs3;

    hs2 = 0;
    for (hs1 = 1; hstr[hs1] != 0; ++hs1){
        hs3 = hstr[hs1];
        if (hs3 >= '0' && hs3 <= '9'){
            hs3 -= '0';
        }else if (hs3 >= 'A' && hs3 <= 'F'){
            hs3 -= ('A'-10);
        }else if (hs3 >= 'a' && hs3 <= 'f'){
            hs3 -= ('a'- 10);
        }else{
            *hsv = 0; return FALSE;
        }
        hs2 = hs2*16+hs3;
    }
    *hsv = hs2;
    if ((hs2 & ~0xffff) != 0){
        return FALSE;
    }else{
        return TRUE;
    }
}

/* =========================>>> LOWER  <<<==============================*/
void lower(str)

char str[];
{
    int i;

    for (i=0; str[i]; ++i){
       str[i]=clower(str[i]);
    }
}

/*=============================>>> UPPER  <<<================================*/
void upper(str)

char str[];
{
    static int i;

    for (i=0; str[i]; ++i){
        str[i]=cupper(str[i]);
    }
}

/* ===============================>>> strcpy <<<============================ */
strcpy(to, from)

unsigned char *to, *from;
{
    /* string copy 1-15-89 removed unneeded unsigned char *cp
    references.  Ya can see, I can be dumb too. */
    for (;*to++ = *from++;)
       {;}
    return TRUE;
}

/* ===============================>>> strlen <<<============================ */
strlen(string)            /* return the length of a string */

unsigned char *string;
{
    unsigned char *cp;

    for (cp=string++;*cp++;)
       {;}
    return cp-string;
}

/* ===============================>>> wtohs <<<============================== */
void wtohs(wtw, wths)        /* word to hex string */

int wtw;
char *wths;
{
    int t1, t2, t3;

    t1 = wtw;
    for (t3 = 3; t3 >= 0; --t3){
        t2 = t1 & 0xf;
        if (t2 < 10){
            t2 += '0';
        }else{
            t2 += ('A'-10);
        }
        wths[t3] = t2;
        t1 = (t1 >> 4) & 0xfff;
    }
    wths[4] = 0;
}
