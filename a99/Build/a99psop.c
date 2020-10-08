/* a99psop.c - This module contains the pseudo op code */

#include <stdio.h>
#include "a99.h"

/* ===============================>>> psop <<<==============================*/
void psop(psost, psdelim)

char *psost;
char psdelim;
{

    /* handle pseudo ops */
    long psv;
    int starttext, firstdata, iptr, sindex, t2, fndtok, optr, done, ps1;
    char psd;

    if (eqlstr(psost, "AORG")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        relflag = FALSE;            /* turn off relative processing */
        if (passnr != 1){
            outcom(line);
        }
        gtok(tok, &psd);
        if (psd != 0 && psd != ' ' && psd != 9){
            p2err("Too many values for AORG");
        }
        if ((t2 = eval(tok, &psv)) != ABSTYP){
            p2err("AORG address cannot be REF");
	}
        if ((psv & 0x1) != 0){
            p2err("AORG must be even address, rounded up");
            ++psv;
        }
        loc = psv;
        genobject = TRUE;           /* object generation ON */
        oboldl = -1;                /* force eject of location */
    }else if (eqlstr(psost, "RORG")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        relflag = TRUE;             /* turn on relative processing */
        if (passnr != 1){
            outcom(line);
        }
        gtok(tok, &psd);
        if (psd != 0 && psd != ' ' && psd != 9){
            p2err("Too many values for RORG");
	}
        if ((t2 = eval(tok, &psv)) == REFTYP){
            p2err("RORG address cannot be REF");
	}
        if ((psv & 0x1) != 0){
            p2err("RORG must be even address, rounded up");
            ++psv;
        }
        loc = psv;
        genobject = TRUE;           /* object generation ON */
        oboldl = -1;                /* force eject of location */

/* DORG - Dummy Origin, do not generate any object code */
    }else if (eqlstr(psost, "DORG")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        if (passnr != 1){
            outcom(line);
        }
        gtok(tok, &psd);
        if (psd != 0 && psd != ' ' && psd != 9){
            p2err("Too many values for DORG");
        }
        if ((t2 = eval(tok, &psv)) != ABSTYP){
            p2err("DORG address cannot be REF");
        }
        if ((psv & 0x1) != 0){
            p2err("DORG must be even address, rounded up");
            ++psv;
        }
        loc = psv;
        genobject = FALSE;          /* Object generation OFF */
        oboldl = 0;

/* EVEN - if location counter currently odd, bumps it by one */
    }else if (eqlstr(psost, "EVEN")){
        if (passnr != 1 && objhel){
            emitbyte(loc, 0);
        }
        loc = ++loc & 0xfffe;
        if (passnr != 1){
            outcom(line);
        }

/* DEF - define an external entry into program */
    }else if (eqlstr(psost, "DEF")){
        if (psdelim != ' ' && psdelim !=9){
            p2errt("Psuedo op must be followed by blank or tab: ", psost);
        }
        firstdata = TRUE;                       /* set first line true */
PSOD:
        gtok(tok, &psd);
        if (passnr != 1){
            if (!(sindex = lookup(tok, &psv))){  /* 1-11-89 removed (long*) */
                p2errt("DEF'd symbol name not found: ", tok);
            }else if (symtyp[sindex] == REFTYP){
                p2errt("DEF'd symbol name cannot be REF: ", tok);
            }else{
                if (firstdata){
                     emlst(loc, psv, line, ' ');   /* emit listing line */
		}else{
                emlst(loc, psv, " ", ' ');         /* emit blank listing line */
                }
                firstdata = FALSE;
                emtdef(tok, psv, symtyp[sindex]);  /* send a def to object file */
            }
        }
        if (psd == ','){        /* Check to see if another DEF name follows */
            if (gtptr > 129){
                p2err("DEF can't end with ','");
            }else{
                goto PSOD;
            }
        }
        if ((psd != 0) && (psd != ' ')){
            p2err("Invalid delimiter in DEF list");
            gtptr = 200;
        }

/* REF - define an absolute external reference */
    }else if (eqlstr(psost, "REF")){
        if (psdelim != ' ' && psdelim !=9){
            p2errt("Psuedo op must be followed by blank or tab: ", psost);
	}
        firstdata = TRUE;           /* haven't emitted line yet */

        /* do for each label in REF list */
PSOR:
        gtok(tok, &psd);
        if (passnr == 1){           /* just enter on pass1 */
            enter(tok, 0, REFTYP);
	}else{                      /* pass 2 */
        sindex = lookup(tok, &psv);      /* 1-11-89 removed (long*)*/
        if (firstdata){
            emlst(0, psv, line, ' ');
        }else{
        emlst(0, psv, " ", ' ');
        }
        firstdata = FALSE;
        }

        /* Check to see if another REF name follows */
        if (psd == ','){
            if (gtptr > 129){
                p2err("REF can't end with ','");
            }else{
            goto PSOR;
            }
        }
        if ((psd != 0) && (psd != ' ')){
            p2err("Invalid delimiter in REF list");
            gtptr = 200;
        }

/* COPY - Get an INCLUDE file name */
    }else if (eqlstr(psost, "COPY")){
        if (passnr != 1){
            emlst(loc, 0, line, ' ');
	}
        if (incflag){
            p2err("Error - COPY file contained COPY directive");
        }else{
        gtok(tok, &psd);
        if (tok[0] == '"'){                  /* argument of file name */
            iptr = 1;
            optr = 0;
            while ((tok[iptr] != '"') && (tok[iptr] != 0) && (iptr<80)){
                copyf[optr++] = tok[iptr++];
	    }
            copyf[optr] = 0;
            incflag = TRUE;
            if (!(chinc = fopen(copyf, OPENRD))){
                printf("Can't open COPY file %s, status=%d\n", copyf, chinc);
                exit(999);
            }
        }else{
            p2err("Bad argument on COPY directive");
        }
        }

/* EQU - define an equated value */
    }else if (eqlstr(psost, "EQU")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
        }
        gtok(tok, &psd);
        if (psd != 0 && psd != ' ' && psd != 9){
            p2err("Too many values for EQU");
	}
        if (labelname[0] == 0){
            if (passnr != 1){
                outcom(line);
            }
            p2err("Label missing from EQU");
        }else if (passnr == 1){             /* just enter on pass1 */
                if ((t2 = eval(tok, &psv)) == ABSTYP){
                    enter(labelname, psv, ABSTYP);
                }else if (t2 == RELTYP){
                    enter(labelname, psv, RELTYP);
                }else{
                    p2err("EQUATE value must be absolute");
                }
        }else{                              /* pass 2 */
        sindex = lookup(labelname, &psv);   /* 1-11-89 removed (long*)*/
        emlst(0, psv, line, ' ');
        t2 = eval(tok, &ps1);
        if (ps1 != psv){
            p2errt("Multiply defined EQUates", labelname);
        }
        if (t2 == REFTYP){
            p2errt("Illegal forward reference on EQUATE: ", labelname);
        }
        }
    }else if (eqlstr(psost, "DATA")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
        }
        firstdata = TRUE;                   /* haven't emitted line yet */
        if (passnr != 1 && objhel){	    /* force an EVEN directive */
            emitbyte(loc, 0);
        }
        loc = ++loc & 0xfffe;

PSOW:
        gtok(tok, &psd);
        ploc = loc;
        t2 = eval(tok, &psv);
        if (passnr != 1){
            if (firstdata){ /* first time need to emit line */
                if (t2 == ABSTYP){
                    emit(loc, psv, line);
                }else if (t2 == RELTYP){
                    emitr(loc, psv, line);
                }else if (relflag & (psv != 0)){
                    emitlref(loc, psv, line);
                }else{
                emit(loc, psv, line);
                }
                firstdata = FALSE;          /* have emitted line */
            }else{
            if (t2 == ABSTYP){
                emit(loc, psv, " ");
            }else if (t2 == RELTYP){
                emitr(loc, psv, " ");
            }else if (relflag & (psv != 0)){
                emitlref(loc, psv, " ");
            }else{
                emit(loc, psv, " ");
            }
            }
        }
        loc += 2;
        if (psd == ','){
            if (gtptr > 129){
                p2err("DATA can't end with ','");
            }else{
                goto PSOW;
            }
	}
        if ((psd != 0) && (psd != ' ')){
            p2err("Invalid delimiter in DATA list");
            gtptr = 200;
        }

/* IFLT  - do LT test on symbol value */
    }else if (eqlstr(psost, "IFLT")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        gtok(tok, &psd);                /* get symbol & value */
        t2 = eval(tok, &psv);
        if (t2 != ABSTYP){
            p2errt("IF operand must be absolute, not REF: ", psost);
	}
        if (iffflag){                   /* if already processing IF */
            p2errt("IF Blocks cannot contain IF's: ", psost);
	}
        iffflag = TRUE;                 /* set processing IF */
        if (psv>=0){                    /* if expression is FALSE */
            inhflag = TRUE;             /* turn off source processing til ELSE, ENDIF */
	}
        if (passnr != 1){               /* display line */
            outcom(line);
        }

/* IFLE  - do LE test on symbol value */
    }else if (eqlstr(psost, "IFLE")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        gtok(tok, &psd);            /* get symbol & value */
        t2 = eval(tok, &psv);
        if (t2 != ABSTYP){
            p2errt("IF operand must be absolute, not REF: ", psost);
	}
        if (iffflag){               /* if already processing IF */
            p2errt("IF Blocks cannot contain IF's: ", psost);
	}
        iffflag = TRUE;             /* set processing IF */
        if (psv>0){                 /* if expression is FALSE */
            inhflag = TRUE;         /* turn off source processing til ELSE, ENDIF */
	}
        if (passnr != 1){           /* display line */
            outcom(line);
	}

/* IFGT  - do GT test on symbol value */
    }else if (eqlstr(psost, "IFGT")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        gtok(tok, &psd);            /* get symbol & value */
        t2 = eval(tok, &psv);
        if (t2 != ABSTYP){
            p2errt("IF operand must be absolute, not REF: ", psost);
	}
        if (iffflag){               /* if already processing IF */
            p2errt("IF Blocks cannot contain IF's: ", psost);
	}
        iffflag = TRUE;             /* set processing IF */
        if (psv<=0){                /* if expression is FALSE */
            inhflag = TRUE; /* turn off source processing til ELSE, ENDIF */
	}
        if (passnr != 1){           /* display line */
            outcom(line);
	}

/* IFGE  - do GE test on symbol value */
    }else if (eqlstr(psost, "IFGE")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        gtok(tok, &psd);            /* get symbol & value */
        t2 = eval(tok, &psv);
        if (t2 != ABSTYP){
            p2errt("IF operand must be absolute, not REF: ", psost);
	}
        if (iffflag){               /* if already processing IF */
            p2errt("IF Blocks cannot contain IF's: ", psost);
	}
        iffflag = TRUE;             /* set processing IF */
        if (psv<0){                 /* if expression is FALSE */
            inhflag = TRUE;         /* turn off source processing til ELSE, ENDIF */
	}
        if (passnr != 1){           /* display line */
                outcom(line);
	}

/* IFEQ   - do EQ test on symbol value */
    }else if (eqlstr(psost, "IFEQ")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        gtok(tok, &psd);            /* get symbol & value */
        t2 = eval(tok, &psv);
        if (t2 != ABSTYP){
            p2errt("IF operand must be absolute, not REF: ", psost);
	}
        if (iffflag){               /* if already processing IF */
            p2errt("IF Blocks cannot contain IF's: ", psost);
	}
        iffflag = TRUE;             /* set processing IF */
        if (psv){                   /* if expression is TRUE */
            inhflag = TRUE; /* turn off source processing til ELSE, ENDIF */
	}
        if (passnr != 1){           /* display line */
            outcom(line);
	}

/* IF/IFNE   - do NE test on symbol value */
    }else if (eqlstr(psost, "IFNE") || eqlstr(psost, "IF")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        gtok(tok, &psd);            /* get symbol & value */
        t2 = eval(tok, &psv);
        if (t2 != ABSTYP){
            p2errt("IF operand must be absolute, not REF: ", psost);
	}
        if (iffflag){               /* if already processing IF */
            p2errt("IF Blocks cannot contain IF's: ", psost);
	}
        iffflag = TRUE;             /* set processing IF */
        if (!psv){                  /* if expression is FALSE */
            inhflag = TRUE;         /* turn off source processing til ELSE, ENDIF */
	}
        if (passnr != 1){           /* display line */
            outcom(line);
	}

/* ELSE - flip processing of IF */
    }else if (eqlstr(psost, "ELSE")){
        if (!iffflag){              /* if not processing IF */
            p2errt("ELSE without preceding IF: ", psost);
	}                           /* 1-10-89 removed commented out line */
        if (passnr != 1){           /* display line */
            outcom(line);
	}

/* ENDIF - Terminate IF processing */
    }else if (eqlstr(psost, "ENDIF")){
        if (!iffflag){
            p2errt("ENDIF statement not preceded by an IF: ", psost);
	}
        iffflag = FALSE;        /* turn off IF processing */
        inhflag = FALSE;        /* turn off inhibit source code processing */
        if (passnr != 1){       /* display line */
            outcom(line);
        }

/* BYTE - generate byte values */
    }else if (eqlstr(psost, "BYTE")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        firstdata = TRUE;       /* haven't emitted line yet */
PSBW:
        gtok(tok, &psd);
        t2 = eval(tok, &psv);
        if (t2 != ABSTYP){
            p2errt("BYTE operand must be absolute: ", psost);
	}
        if (passnr != 1){
            if (firstdata){     /* first time need to emit line */
                emlst(loc, psv, line, ' ');
                firstdata = FALSE;
            }
            emitbyte(loc, psv);
        }
        loc += 1;
        if (psd == ','){
            if (gtptr > 129){
                p2err("BYTE can't end with ','");
            }else{
            goto PSBW;
            }
        }
        if ((psd != 0) && (psd != ' ')){
            p2err("Invalid delimeter in BYTE list");
            gtptr = 200;
        }

/* PAGE - eject new page */
    }else if (eqlstr(psost, "PAGE")){
        if (psdelim != ' ' && psdelim != 9 && psdelim != 0){
            p2errt("Pseudo op must be followed by blank, tab, or eol: ", psost);
	}
        if (passnr != 1){
            outcom(line);
            emtsr("\f");
        }

/* IDT - User identifier, picked up on pass 1 only */

    }else if (eqlstr(psost, "IDT")){
        if (passnr != 1){
            outcom(line);
	}
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
        }
        iptr = 0;
        if (line[gtptr] != QUOTE){
            p2errt("Identifier doesn't start with single quote: ", psost);
        }else{
            gtptr++;
            for (iptr=0; iptr<8; iptr++){
                idt[iptr] = ' ';
	    }
            idt[8] = 0;
            iptr = 0;
            while ((iptr < 8) && (line[gtptr] != QUOTE) && (line[gtptr] != 0)){
                idt[iptr++] = line[gtptr++];
            }
            if (line[gtptr] != QUOTE){
                p2errt("Identifier > 8 chars, or no terminating quote: ", psost);
            }
	}

/*  TITL - Title on top of heading page */
    }else if (eqlstr(psost, "TITL")){
        if (passnr != 1){
            outcom(line);
	}
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        iptr = 0;
        if (line[gtptr] != QUOTE){
            p2errt("Text string doesn't start with single quote: ", psost);
        }else{
        gtptr++;
        for (iptr=0; iptr<MAXTITL; iptr++){
            titl[iptr] = ' ';
        }
        iptr = 0;
        while ((iptr < MAXTITL) && (line[gtptr] != QUOTE) && (line[gtptr] != 0)){
            titl[iptr++] = line[gtptr++];
        }
        if (line[gtptr] != QUOTE){
            p2errt("Title too long, or no terminating quote: ", psost);
        }
        }

/* TEXT statement, text quoted with some character */
    }else if (eqlstr(psost, "TEXT")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        if (passnr != 1){
            outcom(line);           /* show line 1st */
	}
        starttext = loc;            /* start of text address */
        while (line[gtptr] == ' ' || line[gtptr] == 9){
            ++gtptr;
	}
        if (line[gtptr] == 0){
            p2err("String missing for TEXT");
	}else{
        psd = line[gtptr++] & 0x7f; /* beginning delimiter */

                /* while not ending delimiter, and not end of line      */
                /* watch for special occurance of delimiter/delimiter   */
                /* which means to use a single delimiter character      */

        done = FALSE;
        while (!done){
            psv = line[gtptr++] & 0x7f;
            if (psv == 0){
                p2err("Missing trailing delimeter in TEXT");
                done = TRUE;
            }else if (psv == psd){          /* if think I'm on delimeter */
                if (line[gtptr] == psd){ /* if next character is a delimeter */
                    gtptr++;
                }else{
                done = TRUE;                /* else, I am at the end */
                }
	    }
            if (!done){                     /* if not done */
                if (passnr != 1){           /* gen object if not pass 1 */
                    emitbyte(loc, psv);
                    if (listtext){
                        emlst(loc, psv, " ", ' ');
                    }
                }
                loc += 1;                   /* bump location counter by 1 */
            }
        }                                   /* end of while loop */
        }                                   /* end of no error processing */
                                            /* end of TEXT processing */

/* BSS - Block starting with symbol */
    }else if (eqlstr(psost, "BSS")){
        if (psdelim != ' ' && psdelim != 9){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
        }
        if (passnr != 1){
            emlst(loc, 0, line, ' ');
        }
        gtok(tok, &psd);
        if (psd != 0 && psd != ' ' && psd != 9){
            p2err("Too many values for BSS");
	}
        if ((t2 = eval(tok, &psv)) != ABSTYP){
            p2err("BSS value MUST be ABSOLUTE");
        }
        loc += psv;
    }else if (eqlstr(psost, "LIST")){   /* list control */
        if (psdelim != ' ' && psdelim != 9 && psdelim != 0){
            p2errt("Pseudo op must be followed by blank or tab: ", psost);
	}
        if (passnr == 2){
            outcom(line);
	}
        if (psdelim == 0){              /* no args, turn listing ON */
            listst = TRUE;
	}else{
        gtok(tok, &psd);
        if (tok[0] == '"'){             /* argument of file name */
            iptr = 1;
            optr = 0;
            while ((tok[iptr] != '"') && (tok[iptr] != 0) && (iptr<80)){
                listf[optr++] = tok[iptr++];
	    }
            listf[optr] = 0;
            if (passnr != 1){
                fclose(chlst);
                if (!(chlst = fopen(listf, OPENWT))){
                   printf("Can't create listing token %s, file %s, status=%d\n",
                        tok, listf, chlst);
                        exit(999);
                }
            }
            liston = TRUE;
        }else{                          /* Search for MACRO listing option */
        fndtok = TRUE;
        while (fndtok){
            if (eqlstr(tok, "NOTEXT")){
                set2(&listtext, FALSE);
	    }else if (eqlstr(tok, "TEXT")){
                set2(&listtext, TRUE);
            }else if (eqlstr(tok, "MNEVER")){
                set2(&listmacros, MNEVER);
            }else if (eqlstr(tok, "MONCE")){
                set2(&listmacros, MONCE);
            }else if (eqlstr(tok, "MALWAYS")){
                set2(&listmacros, MALWAYS);
            }else if (eqlstr(tok, "MOBJECT")){
                set2(&listmacros, MOBJECT);
            }else{
            p2errt("Invalid item in LIST directive: ", tok);
            gtptr = 200;
            }
            if (psd == ','){
                gtok(tok, &psd);
            }else{
            gtptr = 200;
            }
            fndtok = FALSE;
        }
        }
        }

/*  UNL - turn listing off */
    }else if (eqlstr(psost, "UNL")){
        if (passnr != 1){           /* only disable for pass 2 */
            outcom(line);
            listst = FALSE;
        }

/*  MACRO - define a macro */
    }else if (eqlstr(psost, "MACRO")){
        savemacro();

/*  END - end of program */
    }else if (eqlstr(psost, "END")){
        if (passnr != 1){
            outcom(line);
        }
        atend = TRUE;
    }else{                          /* macro call or invalid op */
    expandmacro(psost, psdelim);
    }
    gtptr = 200;
}
