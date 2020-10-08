/*  a99pass1.c -->  This module contains the initialization module, pass 1
                    associated routines, symbol table lookup routines, and
                    object code routines.                                  */

#include <stdio.h>
#include "a99.h"

#ifdef  UNIX || __ZTC__ || (LATTICE && MSDOS)
#include <time.h>
    char *ctime(long *);
    long t_start, t_finish;
#endif


/* ===============================>>> init <<<============================== */
void init(void)

{
short int i;

    errs = 0;
    ensym = 1;
    enval = 1;

    for (i=0; i<8; i++){            /* Clear user identifier */
      idt[i] = ' ';
    }
    gtptr = 200;
    listtext = FALSE;               /* don't list object for text */
    listmacros = MOBJECT;           /* list macro object & 1st source line */
    macfirst = FALSE;               /* first macro encountered */
    obflg = FALSE;                  /* first object output or not */
    oboldl = -1;                    /* last location output */
    genobject = TRUE;               /* generate object or not */
    nmactxt = 0;                    /* index into next mactxt */
    msymgen[0] = 'a';               /* symbol generator for macro locals */
    msymgen[1] = 'a';               /* symbol generator for macro locals */
    nxtmac = 0;                     /* pointer to next macro to use */
    expanding = FALSE;              /* TRUE if expanding a macro */
    listst  = TRUE;                 /* TRUE until UNL statement */
    incflag = FALSE;                /* not getting input from COPY file */
    iffflag = FALSE;                /* currently processing IF */
    inhflag = FALSE;                /* inhibit processing source or not */
    relflag = TRUE;                 /* assemble relative or not */

    i = strlen(inputf);
    strcpy(&inputf[i],".a99");              /* force .a99 extension */
    if (!(chin = fopen(inputf, OPENRD))){
       printf("\nSource file %s not found - aborting assembly\n", inputf);
       exit(999);
    }

    if (objon){
        i = strlen(objf);
        strcpy(&objf[i], ".x99");            /* force .x99 extension */
        if (!(chobj = fopen(objf, OPENWB))){
            printf("Fatal Error: Can't create hex output file %s, error %d\n",
                 objf, chobj);
            exit (999);
        }
    }

    if (liston && listst){
        i = strlen(listf);
        strcpy(&listf[i], ".l99");          /* copy file name */
        if (!(chlst = fopen(listf, OPENWT))){
            printf ("Fatal Error: Can't create list file %s, error %d\n",
                     listf, chlst);
            exit(999);
        }
    }

#ifdef UNIX || __ZTC__ || (LATTICE && MSDOS)
    time(&t_start);
    printf("Assembly begins: %s", ctime(&t_start));
    if (liston && listst){
      fprintf(chlst, "***** %s *****\n", VERSION);
      fprintf(chlst, "      Assembled on %s\n\n", ctime(&t_start));
    }
#else
    printf("Assembly begins\n");
#endif

    for (i=0; i<MAXVAL; i++){       /* clear symbol table entries */
        symind[i] = 0;              /* 1-11-89 removed extra ";". added {}'s */
    }
    if (registers){                 /* if registers are to be defined */
        enter("r0", 0, ABSTYP);     /* enter register defintions */
        enter("r1", 1, ABSTYP);
        enter("r2", 2, ABSTYP);
        enter("r3", 3, ABSTYP);
        enter("r4", 4, ABSTYP);
        enter("r5", 5, ABSTYP);
        enter("r6", 6, ABSTYP);
        enter("r7", 7, ABSTYP);
        enter("r8", 8, ABSTYP);
        enter("r9", 9, ABSTYP);
        enter("r10", 10, ABSTYP);
        enter("r11", 11, ABSTYP);
        enter("r12", 12, ABSTYP);
        enter("r13", 13, ABSTYP);
        enter("r14", 14, ABSTYP);
        enter("r15", 15, ABSTYP);
    }
}

/* ===============================>>> pass1 <<<==============================*/
pass1(void)

{
int val;
char delim, typ;

    loc = 0x0000;           /* set location counter to >0000 initially */
    atend = FALSE;

P1LP:
    if (atend){                     /* if done processing */
        return TRUE;
    }
    labelname[0] = 0;
    if (!gtok(tok, &delim)){
        return TRUE;                /* eof => end */
    }
    while (delim == ':'){           /* label? */
        strcpy(labelname, tok);
        gtok(tok, &delim);          /* might be equ */
        if (eqlstr(tok, "EQU") || eqlstr(tok, "MACRO")){
            typ = 'P';
            goto P1EQU;
        }
        if (relflag){
            enter(labelname, loc, RELTYP);
        }
        else{
            enter(labelname, loc, ABSTYP);
        }
    }

/* to here when token must be an opcode -                       */
/* get op code and find out its type, increment loc counter by  */
/* proper amount                                                */

    gopcod(tok, &typ, &val);            /* get op code */

P1EQU:
    if (typ == 'P'){
        psop(tok, delim);               /* go handle pseudo op */
        goto P1LP;
    }

    loc = ++loc & 0xfffe;
    loc += 2;                           /* all instructions take >= 1 word */
    if (typ == 'I' || typ == 'R'){
        loc += 2;                       /* i and r take 1 more word */
    }
    else if (typ == 'D'){               /* doubles may take more space */
        gtok(tok, &delim);              /* fetch 1st operand */
        if (tok[0] == '@'){
            loc += 2;                   /* one more if symbolic */
        }
        if (delim == ','){              /* must be ',', otherwise pass2 err */
            gtok(tok, &delim);          /* fetch 2nd operand */
            if (tok[0] == '@'){
                loc += 2;               /* one more if symbolic */
            }
        }
    }
    else if (typ == 'O' || typ == 'N' || typ == 'W'){
        gtok(tok, &delim);              /* may take some space */
        if (tok[0] == '@'){
            loc += 2;                   /* one more if symbolic */
        }
    }
    gtptr = 200;                        /* force new read line */
    goto P1LP;
}

/* ===============================>>> aquit <<<==============================*/
void aquit(void)

{
char elfs[6];
short int i;

    fclose(chin); fclose(chlst);
    if (genobject){                 /* if object generation ON */
        if (objhel){                /* force an EVEN statement */
            emitbyte (loc, 0);
        }
        loc += 1;
    }
    emtref();                       /* force all REF's to be generated */
    flushobj();                     /* flush the object file record */
    if (objon){                     /* write last record, EOF (:) */
        strcpy (objline, ":   99/4A and 9640 Cross-Assembler");
        for (i=34; i<76; i++){      /* blank to sequence number */
            objline[i] = ' ';
	}
        sprintf (elfs, "%04d", objseq);     /* sequence number */
        objseq++;
        for (i=76; i<80; i++){
         objline[i] = elfs[i-76];
	}
        objline[80] = 0;            /* terminating null */
	if ( objon ) 
          {
          fprintf (chobj, "%s\n", objline);
          fclose(chobj);
	  }
    }

#ifdef UNIX || __ZTC__ || (LATTICE && MSDOS) /* 1-21-89 Makes timings easier */
    time(&t_finish);
    printf("\nAssembler done: %s", ctime(&t_finish));
    printf("Elapsed time for assembler: %.0f seconds.\n",
    		difftime(t_start, t_finish));
    printf("Errors: %d\n\n", errs);
#else
    printf("\nAssembler done.  Errors: %d\n\n", errs);
#endif

}

/* ===============================>>> ckreg <<<==============================*/
void ckreg(ckr)

int *ckr;
{
    if (*ckr < 0 || *ckr > 15){
       p2err("Invalid register, not between 0 and 15");
    }
    *ckr = *ckr & 0xf;
}

/* ===============================>>> emit <<<==============================*/
void emit(emloc, emval, elin)

int emloc, emval;
char *elin;
{
    emlst(emloc, emval, elin,' ');      /* emit listing line */
    emtobj(emloc, emval, 'B');          /* emit to object code file */
}

/* ===============================>>> emitlref <<<==============================*/
void emitlref(emloc, emval, elin)

int emloc, emval;
char *elin;
{
    emlst(emloc, emval, elin, '\'');    /* emit listing line */
    if ( xopcode )
      emtobj(emloc, emval, 'X');          /* emit to object code file */
    else
      emtobj(emloc, emval, 'C');	  /* use normal relative REF */
}
/* ===============================>>> emitr <<<==============================*/
void emitr(emloc, emval, elin)

int emloc, emval;
char *elin;
{
    emlst(emloc, emval, elin, '\'');    /* emit listing line */
    emtobj(emloc, emval, 'C');          /* emit to object code file */
}

/* =============================>>> emitbyte <<<=============================*/
emitbyte(ebloc, ebval)

int ebloc, ebval;
{
    if (ebloc == 0){                    /* if just an equate, return */
        return TRUE;
    }

/* if object held, and this location one plus the last */
    if (objhel && (ebloc == objloc+1)){
        objhel = FALSE;
        emtobj(objloc, ebval+objbyt, 'B');
        objbyt = 0;
        return TRUE;
    }else if (objhel){                  /* if object held, and new next, */
      emtobj(objloc, objbyt, 'B');      /* then just output it */
    }
    objhel = TRUE;
    objloc = ebloc;
    objbyt = ebval << 8;                /* even loc, save old byte */
}

/* ===============================>>> emlst <<<==============================*/
/*                                                                           */
/* emlst - prints a listing line.  It is passed the location (ell), the      */
/*         value (elv), and the line string (elli).                          */
/*                                                                           */
/*         Object code is flagged with the single character passed (emflag)  */
/*         Tabs are expanded into spaces (8 spaces per tab), note that       */
/*         each line starts in column 21.                                    */
/* ==========================================================================*/
emlst(ell, elv, elli, emflag)

int ell, elv;
char *elli;
char emflag;                                /* 1-10-89 changed int to char */
{
    char emstr[6], strout[6];
    int  colcnt;

    if (expanding){                         /* decide if want to return */
        if (listmacros == MNEVER){          /* return if never */
           return TRUE;
        }
        if (listmacros == MONCE){
            if (!macfirst){
               return TRUE;                 /* return if not first time */
            }
        }
    }
    chkpage();
    if (listst && liston ){
        sprintf (strout, "%4d", alineno);
    }
    emtsr(strout);
    emtsr(" ");
    wtohs(ell, emstr);                      /* convert to hex string */
    emtsr(emstr);                           /* emit location */
    wtohs(elv, emstr);
    emtsr("   ");
    emtsr(emstr);                           /* emit value */
    strout[0] = emflag;
    strout[1] = ' ';                        /* relative or not flag */
    strout[2] = ' ';
    strout[3] = ' ';
    strout[4] = ' ';
    strout[5] = 0;
    emtsr(strout);

/* if macro expansion, and list just object */
    if (expanding && (listmacros == MOBJECT)){
        emtsr(mline);
        mline[0] = 0;
    }else{
        if (listst && liston ){
            colcnt = 0;                     /* listings start at col 21 */
            strout[0] = ' ';
            strout[1] = 0;
            for (; *elli; ++elli)
                if (*elli == 9){
another:
                    emtsr(strout);
                    colcnt++;
                        if ((colcnt/8) * 8 != colcnt){
                            goto another;
                        }
                }else{
                    fputc(*elli, chlst);
                    colcnt++;
                }
        }
    }
    emtsr("\n");
}

/* ===============================>>> emtobj <<<==============================*/
void emtobj(emol, emov, emty)

int emol, emov, emty;
{
    if (!obflg){                        /* first time through */
        emtlv(emol);                    /* emit location */
        emtv(emol, emov, emty);         /* emit object */
        obflg = TRUE;
    }else{
        if (objhel){                    /* if object being held (byte) */
            objhel = FALSE;             /* turn it off */
            emtlv(objloc);              /* emit location */
            emtv(objloc, objbyt, emty); /* emit value */
            emtlv(emol);                /* emit location */
            emtv(emol, emov, emty);     /* emit value */
        }else if (emol != oboldl+2){    /* skipped over a block */
            emtlv(emol);
            emtv(emol, emov, emty);
        }else{                          /* next sequential val */
            emtv(emol, emov, emty);
        }
    }
    oboldl = emol;                      /* so can emit correctly next time */
}

/* ===============================>>> emtdef <<<============================*/
void emtdef(deflbl, defval, deftyp)

/*  emtdef - emit a define into the current object.  Note that the
    true TI assembler emits all defines at the end, I find it easier
    to emit defines as I go along (there's no rule that says the linker
    can't handle a define in the middle of the object code).

    Modified on 7/29/90 to emit long defines if we are emitting long
    defs (using extended assembler).

                                                                       */
char deflbl[31];
int  deftyp, defval;
{
short deflen;
    /* emit a DEF into the object code */
    char elvs[6];
    short int i, blaflg;

    if (objhel)	{			/* if an object waiting to go */
      objhel = FALSE;			/* send it */
      if (deftyp == ABSTYP)
          emtv(objloc,objbyt,'B');
      else
          emtv(objloc,objbyt,'C');
      }

/*    make sure that the new object will fit on line */
    deflen = strlen( deflbl );

    if ( deflen < 6 )
      deflen = 6;

    if ( compobj )
        {
        if ( objptr > ( NEWCLIN - (deflen+3)) )
            flushobj();
	}
    else
        {
        if ( objptr > ( NEWOLIN - (deflen+5)) )
            flushobj();
	}

    if (deftyp == RELTYP)
        objline[objptr++] = '5';        /* External Def, relative */
    else
        objline[objptr++] = '6';	/* External Def, absolute */

    if ( compobj )
      {
      objline[objptr++] = (defval >> 8) & 0xff;
      objline[objptr++] = defval & 0xff;
      }
    else
      {
      wtohs(defval,elvs);		/* convert location to ASCII   */
      for (i = 0; i < 4; ++i)           /* move hex location to object */
          objline[objptr++] = elvs[i];
      }

    blaflg = FALSE;
    deflen = strlen ( deflbl );
    if ( deflen < 7 || !extended )
      {
      for (i = 0; i < 6; ++i)		/* move 6 character def name to obj */
        {
        if ( deflbl[i] == 0 )
          blaflg = TRUE;

        if (!blaflg)			/* pad to right with blanks */
          {
          objline[objptr++] = cupper(deflbl[i]);
          }
        else
          {
          objline[objptr++] = ' ';
	  }
	}
      }
    else
      {
      objline[objptr++] = deflen;	/* encode default length */
      for ( i=0; i < deflen; ++i)	/* move 31 character label to obj */
	objline[objptr++] = cupper(deflbl[i]);
      }
}
/* ===============================>>> emtref <<<==============================*/
void emtref(void)

{
    /* emtref scans the symbol table, search for symbols of type REFSYM,
       and outputting them to the object file.                         */

    /* This modified on 18/feb/88 to not emit refs that are not used in
       the body of the program.  Problem was these were resolved to loc
       0 relative in the program, causing overwrites at link time      */

    /* This modified on 08/Jul/90 to allow references longer than 6 bytes
       if the extended assembler flag is set.  The loader has similar
       modifications to accept references of this longer length.

       A reference of length 7 to 31 characters has the length encoded
       in the first byte, followed by the string.  Less than or equal
       to six bytes is encoded the old way, with 1-6 characters blank
       padded to the right.                                            */

    int vindex, tindex, symlen, refptr;
    short i;
    char reft[33], elvs[6];

    vindex = 0;				/* start pointer */
    while ( ++vindex <= MAXVAL ) {
      if ( (symind[vindex] !=0) && (symtyp[vindex] == REFTYP)){
        tindex = symind[vindex];		/* get pointer to symbol */
        symlen = strlen( &symt[tindex] );	/* get length of symbol */
        refptr = 0;

        if ( !symval[vindex] ) {
/*          printf("Warning - Symbol %s REF'd but not used\n",&symt[tindex]);*/
	}else{

/* if symbol length 6 chars or less, or not extended assembler, then
   must generate the old way (no more than 6 character ref/defs) */
          
	if ( symlen <7 || !extended ) {
	    strcpy( &reft[0], &symt[tindex] );
	    while ( symlen < 6 )
	      reft[symlen++] = ' ';
	    symlen = 6;
	    reft[6] = 0;

/* else, symbol is greater than 6 chars, and we are running with the
   extended assembler.   Set the length in the first byte, and copy
   the string to the remaining */
	  }else{
	    reft[0] = symlen;
	    strcpy ( &reft[1], &symt[tindex] );
	  }

/*   make sure the symbol will fit on the object line */

        if ( compobj )
           {
           if ( objptr > (NEWCLIN - (symlen+3)))
	     flushobj();
	   }
	else
           {
           if ( objptr > (NEWOLIN - (symlen+5)) )
             flushobj();
	   }

        if (!relflag){         /* if not generating relative object */
           objline[objptr++] = '4';    /* External Ref, absolute */
        }else{
           objline[objptr++] = '3';    /* External Ref, relative */
	}
	if ( compobj ){
            objline[objptr++] = (symval[vindex] >> 8) & 0xff;
            objline[objptr++] = symval[vindex]        & 0xff;
	}else{
            wtohs ( symval[vindex], elvs );
            for (i = 0; i < 4; ++i){    /* move hex chain address to object */
                objline[objptr++] = elvs[i];
	    }
	}
 	symlen = strlen ( reft );
	strcpy ( &objline[objptr], &reft[0] );
	objptr += symlen;
      }
      }
    }
}

/* ===============================>>> emtlv <<<==============================*/
void emtlv(elvl)

int elvl;
{
    /* emit location */
    int  i;
    char elvs[6];

    if (genobject){                     /* if object generation ON */
        if ( compobj ){
            if ( objptr > NEWCLIN - 3 ){
                flushobj();
            }
	}else{
            if ( objptr > NEWOLIN - 5 ){
                flushobj();
	    }
	}

        wtohs(elvl, elvs);
        if (relflag){
            objline[objptr++] = 'A';    /* Load relative address */
        }else{
        objline[objptr++] = '9';        /* Load absolute address */
	}
        if ( compobj ){
            objline[objptr++] = (elvl >> 8 ) & 0xff;
            objline[objptr++] = elvl         & 0xff;
  	}else{
            wtohs(elvl, elvs);
            for (i = 0; i < 4; ++i){
                objline[objptr++] = elvs[i];
	    }
	}
    }
}

/* ===============================>>> emtv <<<==============================*/
void emtv(elll, evvl, evty)

int elll, evvl, evty;

/* emit value */
{
    char elvs[6];
    short int i;

    if (genobject){                     /* if object generation ON */
        if ( compobj ){
            if (objptr > NEWCLIN-3 ){
                flushobj();
	    }
	}else{
            if (objptr > NEWOLIN-5 ){
                flushobj();
	    }
	}
        objline[objptr++] = evty;       /* Type of data to generate */
	if ( compobj ){
            objline[objptr++] = ( evvl >> 8 ) & 0xff;
            objline[objptr++] = evvl          & 0xff;
	}else{
            wtohs ( evvl, elvs );
            for (i = 0; i < 4; ++i){
                objline[objptr++] = elvs[i];
	    }
	}
    }
}

/* ==============================>>> flushobj <<<============================*/
void flushobj(void)

{
    char elfs[6];
    short int i;

    if (objptr > 0 ){
        if ( compobj ){
            objline[objptr++] = 'F';           /* end of line object tag */
            for ( ; objptr<79; objptr++ ){
                objline[objptr] = ' ';
	    }

/*  if compressed object mode, then send characters to file one at a time,
    since a null (which is a string terminator) could easily be used in
    the object record                                                     */

           if ( objon ) {
	       while ( objptr < 80 ){
                   objline[objptr++] = ' ';    /* clear rest with blanks */
	       }
	       for ( i=0; i<80; i++ ){
                   fputc(objline[i],chobj);
	       }
	   }
           objptr = 0;

/* uncompressed object mode.  compute checksum and sequence number,
   insert into object record and write whole record                     */

	}else{
            objline[objptr++] = '7';            /* Checksum tag */
            chksum = 0;
            for (i=0; i!=objptr; i++){
                chksum += objline[i];
	    }
            chksum = -chksum;                   /* two's complement */
            chksum = chksum & 0xffff;
            wtohs(chksum, elfs);                /* to text */
            for (i=0; i<4; ++i){
                objline[objptr++] = elfs[i];
	    }
            objline[objptr++] = 'F';            /* end of record tag */
            for (i=objptr; i<76; i++){          /* blank to sequence number */
                objline[i] = ' ';
   	    }
            sprintf (elfs, "%04d", objseq);     /* sequence number */
            objseq++;
            for (i=76; i<80; i++){
                objline[i] = elfs[i-76];
	    }
            objline[80] = 0;                    /* terminating null */
            if ( objon ){
	       for ( i=0; i<80; i++ ){
                   fputc(objline[i],chobj);
	       }
	    }
            objptr = 0;
	}
    }
}

/* ===============================>>> emtsr <<<==============================*/
void emtsr(exx)
char *exx;
{
    if (listst && liston){
        for (; *exx; ++exx){
            fputc(*exx,chlst);
        }
    }
}
/* ===============================>>> emval <<<==============================*/
void emval(emv1)

int emv1;
{
    /* emit value absolute */
    loc += 2;
    emit(loc, emv1, " ");
}

/* ===============================>>> emvalref <<<==============================*/
void emvalref(emv1)
int emv1;
{
    /* emit value relative */
    loc += 2;
    emitlref(loc, emv1, " ");
}

/* ===============================>>> emvalr <<<==============================*/
void emvalr(emv1)   /* 1-19-89  Why two functions when they are the same */
                    /* emval and emvalr */
int emv1;
{
    /* emit value relative */
    loc += 2;
    emitr(loc, emv1, " ");
}

/* ===============================>>> endp1 <<<==============================*/
void endp1(void)

{
    /* Print location at end of pass1, in hex please */

    if (objhel){           /* if odd byte last, then make it even */
        loc += 1;
    }
    printf("\nEnd of Pass 1 -- Last location used: >%x \n",loc);
}

/* ===============================>>> enter <<<==============================*/
void enter(ensy,env,entt)

char *ensy;
int env;
char entt;                      /* 1-10-89 changed into to char */
{
    int  savstart, hash, i; /* enter the value, then the symbol           */
                            /* ensym is index of start of next new symbol */
                            /* enval is next value                        */
                            /* entt  is the symbol type                   */

    hash = 0;
    for (i=0; ensy[i] !=0; i++){
      hash = hash * 1235 + cupper(ensy[i]);
    }
    if (hash < 0){
        hash = -hash;
    }
    hash = (hash % MAXVAL) + 1;
    savstart = hash;
    while ((symind[hash] != 0) && (hash <= MAXVAL)){
      hash++;
    }
    if (hash > MAXVAL){
        hash = 1;
        while ((symind[hash] != 0) && (hash < savstart)){
            hash++;
	}
        if (hash == savstart){
            printf("Fatal Error: Out of symbol table space\n");
            exit(999);
        }
    }

    /* found an empty slot in location hash, fill it up */

    symval[hash] = env;     /* value of symbol */
    symtyp[hash] = entt;    /* symbol type */
    symind[hash] = ensym;   /* index into symbol text table */

    for (; *ensy; ++ensy){
       symt[ensym++] = cupper(*ensy);   /* upper on the way in */
    }
    symt[ensym++] = 0;
}


/* ===============================>>> eval <<<==============================*/
eval(evst, evl)

char *evst;
int *evl;
{
    /* evaluate the string in evst, return value in evl */
    /* return TRUE if ok, false if can't eval */

    char evtok[MAXTOK];
    char evold, evnew, t;
    int e1, evptr, everr, evfnd, sindex, evt;
    long evsum, evtrm;

    everr = FALSE;  /* assume no error                                      */
                    /* To here, then have general arith expression          */
                    /* evptr points to position in evst                     */
                    /* evsum is the sum of the experession                  */
                    /* evold holds previous arith operator,                 */
                    /* evnew the next one                                   */
                    /* evfnd tells whether a non-absolute expression        */
                    /*       has been found                                 */
                    /* evt   type of expression (1=absolute, 2=ref, 3=rel)  */
    evptr = 0;
    evsum = 0;
    evold = '+';
    evt = ABSTYP;
    evfnd = FALSE;

 /* handle unary - */
    if (evst[evptr] == '-'){
        evold = '-';
        ++evptr;
    }
    while (TRUE){
        e1 = 0;
EVL1:
        t = evst[evptr];
        if (t == '|' || t == '&' || t=='+' || t=='-' || t=='*' || t=='/'){
            goto EVL2;
        }else if (t == '\''){                   /* v1.13 bug fix */
            evtok[e1++] = t;
            evtok[e1++] = evst[++evptr];        /* the character */
            if ((evtok[e1++] = evst[++evptr]) != '\''){      /* 'xx' */
                evtok[e1++] = evst[++evptr];
            }
            t = evst[++evptr];      /* and the terminator char */
            if (e1 >= MAXTOK){
                printf("Fatal Error: Token too big\n");
                exit(999);
            }
        }else if (t != 0){          /* no delimiter yet, so build token */
            evtok[e1++] = t;
            if (e1 > MAXTOK){
                printf("Fatal Error: Token too big\n");
                exit(999);
            }
            ++evptr;
            goto EVL1;
        }

        /* Have a token to evaluate */
EVL2:
        evtok[e1] = 0;                      /* terminate string */
        ++evptr;
        evnew = t;
        if (!*evtok){
            p2errt("Badly formed expression, can't evaluate: ",evst);
            *evl = 0;
            return FALSE;
        }

    /* first, check for characters  (this code moved here for v1.12)*/
        if (evtok[0] == '\''){              /* 'c' or 'cc' */
            if (evtok[2] != '\''){          /* 'cc' */
                if (evtok[3] != '\''){      /* look for terminating ' */
                    p2errt("Expecting chars of form 'cc': ", evtok);
                    everr = TRUE;           /* error */
                }else{
                    evtrm = (evtok[1]<<8) | evtok[2];
                }
                }else{                      /* must be 'c' */
                    evtrm = evtok[1];
                }
        }else if (evtok[0] >= '0' && evtok[0] <= '9'){  /* have a decimal number */
            if (!dstow(evtok, &evtrm)){
                p2errt("Invalid decimal constant: ",evtok);
                everr = TRUE;               /* error */
            }
        }else if (evtok[0] == '>'){                  /* have a hex number */
            if (!hstow(evtok,&evtrm)){
                p2errt("Invalid hex constant: ",evtok);
                everr = TRUE;               /* error */
            }
        }else if (evtok[0] == '$' && evtok[1] == 0){ /* location counter */
            evtrm = loc;
            if (relflag && !evfnd){
                evt = RELTYP;               /* relative object */
                evfnd = TRUE;               /* set found relative once */
            }else{
                evt = ABSTYP;
            }
        }else{                              /* must be a symbol */
            if (!(sindex = lookup(evtok,&evtrm))){
                p2errt("Undefined symbol: ", evtok);
                everr = TRUE;                       /* error */
            }else{

            if (symtyp[sindex] != ABSTYP && !evfnd){
                evt = symtyp[sindex];
                evfnd = TRUE;
            }else{
                evt = ABSTYP;
	    }
            if (symtyp[sindex] == REFTYP){
                if (passnr != 1){
                    symval[sindex] = ploc;
                }
            }
            }
        }

    /* evtrm has value of current term */

        if (evold == '+')
            evsum += evtrm;
        else if (evold == '-')
            evsum -= evtrm;
        else if (evold == '*')
            evsum *= evtrm;
        else if (evold == '/')
            evsum /= evtrm;
        else if (evold == '|')
            evsum |= evtrm;
        else if (evold == '&')
            evsum &= evtrm;
        else{
            p2errt("Badly formed expression, can't evaluate: ",evst);
            everr = TRUE;           /* error */
        }

        evold = evnew;
        if (everr){
            *evl = 0;
            return FALSE;
        }
        if (evnew == 0){            /* done */
            *evl = evsum;           /* return expression sum */
            return evt;             /*        and evaluated type */
        }
    }
}

/* ===============================>>> g015 <<<==============================*/
void g015(g0v,g0d)

int *g0v;
char *g0d;
{
    int t2=ABSTYP;

    /* get a value between 0 and 15 */

    char td;
    int tv;

    gtok(tok, &td);
    *g0d = td;
    if (!(t2 = eval(tok,&tv))){
        *g0v = 0;
    }
    if (t2 != ABSTYP){
        p2err("Evaluated to non-absolute - 0 used");
        *g0v = 0;
    }else{
        *g0v = tv;
        if (*g0v < 0 || *g0v > 15){
            p2err("Value between 0 and 15 required - value mod 15 used");
            *g0v = *g0v & 0xf;
        }
    }
}

/* ===============================>>> gopnd <<<==============================*/
gopnd(gdv, gdf, gdov, gdd, gdot)

int *gdv;            /* value */
char *gdf;           /* flag? */
int *gdov;           /* operand value */
char *gdd;           /* delimiter */
int *gdot;           /* type of symbol (abs, rel) */
{
    int gd1,gd2,gdreg;
    int t1, t2;
    char tc;

    /* get the next token */

    gtok(tok,&tc);
    *gdd = tc;
    *gdf = 'N';
    gd1 = strlen(tok) - 1;          /* gd1 points to last char in tok */
    if (tok[0] == '@'){             /* either symbolic or indexed */
        *gdf = 'Y';
        gdreg = 0;
        if (tok[gd1] == ')'){       /* indexed addressing */
            tok[gd1] = 0;           /* kill) */
GDL1:
            --gd1;
            if (gd1 <= 0){
                p2errt("Badly formed indexed addressing: ",tok);
                *gdv = *gdov = 0;
                return TRUE;
            }
            if (tok[gd1] == '('){
                tok[gd1] = 0;
                gd1++;
                if ((!(t2=eval(&tok[gd1],&gdreg))) || (t2 != ABSTYP)){
                    p2errt("Badly formed indexed addressing: ",tok);
                    *gdv = *gdov = 0;
                    return TRUE;
                    }
                if (gdreg == 0){
                   p2err("WARNING: Using R0 is really symbolic addressing");
		}
	    }else{
                goto GDL1;          /* get whole index */
            }
        } /* end if) */

        /* gdreg has the register, now get the offset or symbol */

        ploc += 2;
        if (!(t2=eval(&tok[1],&t1))){
            *gdv = *gdov = 0;
            return TRUE;
        }
        *gdov = t1;                 /* symbol value */
        *gdot = t2;                 /* symbol type */
        ckreg(&gdreg);
        *gdv = gdreg | 32;
    }else if (tok[0] == '*'){       /* indirect addressing */
        gd2 = 16;
        if (tok[gd1] == '+'){       /* auto increment */
            tok[gd1] = 0;
            gd2 = 48;
        }
        /* have either direct or autoincrement T in gd2 */
        if (!(t2=eval(&tok[1],&gdreg))){
            *gdv = *gdov = 0;
            return TRUE;
        }
        ckreg(&gdreg);
        *gdv = gdreg | gd2;
    }else{                          /* must be a register */
        if (!(t2=eval(tok,&gdreg))){
            *gdv = *gdov = 0;
            return TRUE;
        }
        ckreg(&gdreg);
        *gdv = gdreg;
    }
}

/* ===============================>>> gopcod <<<==============================*/
gopcod(gopc, gopt, gopv)

char *gopc;                  /* op code in */
char *gopt;                  /* op code type out */
int *gopv;                   /* op code value out */
{
    short int gi, gj, bottom, top, gk;
    static char opcod[MAXOPC][7] =
        {
        "A   D ", "AB  D ", "ABS O ", "AI  I ", "AM  Gh",
        "ANDII ", "AR  Oh", "B   O ", "BINDOh", "BL  O ",
        "BLSKIh", "BLWPO ", "C   D ", "CB  D ", "CER Ah",
        "CI  I ", "CIR Oh", "CLR O ", "COC W ", "CR  Fh",
        "CRE Ah", "CRI Ah", "CZC W ", "DEC O ", "DECTO ",
        "DIV W ", "DIVSOg", "DR  Oh", "IDLEA ", "INC O ",
        "INCTO ", "INV O ", "JEQ J ", "JGT J ", "JH  J ",
        "JHE J ", "JL  J ", "JLE J ", "JLT J ", "JMP J ",
        "JNC J ", "JNE J ", "JNO J ", "JOC J ", "JOP J ",
        "LDCRN ", "LDD Ah", "LDS Ah", "LI  I ", "LIMIR ",
        "LR  Oh", "LST Zg", "LWP Zg", "LWPIR ", "MM  Fh",
        "MOV D ", "MOVBD ", "MPY W ", "MPYSOg", "MR  Oh",
        "NEG O ", "NEGRAh", "NOP A ", "ORI I ", "RET A ",
        "RT  A ",
        "RTWPA ", "RSETA ", "S   D ", "SB  D ", "SBO C ",
        "SBZ C ", "SETOO ", "SLA S ", "SLAMMh", "SM  Gh",
        "SOC D ", "SOCBD ", "SR  Oh", "SRA S ", "SRAMMh",
        "SRC S ", "SRL S ", "STCRN ", "STR Oh", "STSTZ ",
        "STWPZ ", "SWPBO ", "SZC D ", "SZCBD ", "TB  C ",
        "TCMBBh", "TMB Bh", "TSMBBh", "U5  A ", "U6  A ",
        "U7  A ", "X   O ", "XOP N ", "XOR W ", "RT  A "
        };

    static short int opval[MAXOPC] =
        {
        0xa000, 0xb000, 1856,   544,    0x002a,
        576,    0x0c10, 1088,   0x0140, 1664,
        0x00b0, 1024,   0x8000, 0x9000, 0x0006,
        640,    0x0c80, 1216,   8192,   0x0301,
        0X0004, 0x0000, 9216,   1536,   1600,
        15360,  0x0180, 0x0d40, 832,    1408,
        1472,   1344,   4864,   5376,   6912,
        5120,   6656,   4608,   4352,   4096,
        5888,   5632,   6400,   6144,   7168,
        12288,  0x07c0, 0x0780, 512,    768,
        0x0d80, 0x0080, 0x0090, 736,    0x0302,
        0xc000, 0xd000, 14336,  0x01c0, 0x0d00,
        1280,   0x0002, 4096,   608,    1115,
        0x045b,
        896,    864,    24576,  28672,  7424,
        7680,   1792,   2560,   0x001d, 0x0029,
        0xe000, 0xf000, 0x0cc0, 2048,   0x001c,
        2816,   2304,   13312,  0x0dc0, 704,
        672,    1728,   16384,  20480,  7936,
        0x0c0a, 0x0c09, 0x0c0b, 928,    960,
        992,    1152,   11264,  10240,  0x045b
        };

    /* search list for match of opcode */
    /* Binary search implemented 3/3/88 a.l.beard */

    bottom = 0;
    top    = MAXOPC;
    gi     = bottom;
    while ((gk = top - bottom)){
        gi = (gk / 2) + bottom;

        /* test to see if this opcode matches the desired */

        for (gj = 0; gj <= 3 && gopc[gj] != 0 ; ++gj){ /* scan 4 letters */
            if (opcod[gi][gj] != cupper(gopc[gj]))
                goto GOP3;
        }

        /* fall through means possible found match */
        if ((gopc[gj] == 0 && opcod[gi][gj] == ' ') || gj == 4){
            /* handle TMS9900 and TMS99000 exclusions */
            if (opcod[gi][5] == 'g' && !(ex9900+ex99000)){
                goto GOP4;
	    }
            if (opcod[gi][5] == 'h' && !ex99000){
                goto GOP4;
            }
            *gopt = opcod[gi][4];       /* return op code type */
            *gopv = opval[gi];          /* and the op code value */
            return TRUE;
        }

GOP3:
        gi = gi;            /* compiler bug? */
        if (gk == 1){            /* next to last time? */
            if (gi = bottom){
                bottom = top;
            }else{
                top    = bottom;
            }
        }else if (opcod[gi][gj] < cupper(gopc[gj])){
            bottom = gi;
        }else{
            top = gi;
	}
    } /* end for gi */

    /* fall through means no match, so assume pseudo op or macro */

GOP4:
    *gopt = 'P';
    *gopv = 0;
}


/* ===============================>>> greg <<<==============================*/
void greg(grg, grd)

int *grg;
char *grd;
{
    int tr,t2;
    char td;

    gtok(tok, &td);
    *grd = td;
    if (!(t2=eval(tok,&tr))){
       *grg = 0;
    }
    ckreg(&tr);
    *grg = tr;
}

/* ===============================>>> gtok <<<==============================*/
gtok(gt, gtd)

char *gt,*gtd;
{
    /* get next token ...
       Skip to next non-delimiter, move token to gt until next delim.
       Logical delims are: :, ' ', ',', end of line
       Actual delims are: above + tab and ';'
       gtok skips over comments                 */

    int gti, gtoks, gtstart;
    char c, delim;
    static char oldd = 0;

GTL0:
    if (gtptr > 129){           /* read new line */
        if (!newlin()){
           return FALSE;
        }
        gtptr = 0;
        gtoks = 0;
    }

/*  Skip over leading delimiters   */
    c = line[gtptr];
    while (c == 9 || c == ' '){
        c = line[++gtptr];
    }
/* Comment if semicolon anywhere, if end of line, or asterick and
   at the beginning of the line (column 1)                        */

    if (c == ';' || c == 0 || ((c == '*') && (gtptr == 0))){
        if (passnr == 2 && (gtoks == 0 || oldd == ':')){
            outcom(line);       /* output comment or label only */
	}
        oldd = 0;
        gtptr = 200;
        goto GTL0;
    }

/* See if inhibiting object generation (due to IF).  If so, then the
   only way out of it is to see an ELSE or ENDIF statement.     */

    if (line[gtptr]   == 'E' && line[gtptr+1] == 'L' &&
        line[gtptr+2] == 'S' && line[gtptr+3] == 'E'){
        inhflag = !inhflag;
	}
    if (inhflag &&
        line[gtptr] == 'E' && line[gtptr+1] == 'N' &&
        line[gtptr+2] == 'D' && line[gtptr+3] == 'I' &&
        line[gtptr+4] == 'F'){
        inhflag = FALSE;
        }


/*  If still inhibiting code generation after above checks, then treat
    everything like a comment line                */

    if (inhflag){
        if (passnr == 2 && (gtoks == 0 || oldd == ':')){
           outcom(line);        /* output comment or label only */
        }
        oldd = 0;
        gtptr = 200;
        goto GTL0;
    }

/*  Aha.  Good code generation now going on.  */
/* have 1st non delimiter */

    gtstart = gtptr;            /* see if starting in column 0 */
    gti = 0;
    if (line[gtptr] == '"' || line[gtptr] == '\''){     /* string ? */
        delim = line[gtptr];    /* remember delim */
        for (gt[gti++] = line[gtptr++];
            line[gtptr] && line[gtptr] != delim; ++gtptr){
            gt[gti++] = line[gtptr];
	    }
        gt[gti++] = delim;
        if (line[gtptr] != delim){
            c = line[gtptr];
            p2err("Character constant missing trailing \" or '");
        }else{
           c = line[++gtptr];   /* next char is delimiter */
	}
        if (c == 9){
            c = ' ';
        }
        else if (c == ';'){
            gtptr = 200;
            c = 0;
        }
    }
GTL2:
    c = line[gtptr];
    if (c == ':' || c == ' ' || c == ','){
        c = c;                  /* no op */
    }else if (c == 0){
        gtptr = 200;
    }else if (c == 9){
        c = ' ';
    }else if (c == ';'){
        gtptr = 200;
        c = 0;
    }else{
        gt[gti++] = c;
        ++gtptr;
        goto GTL2;
    }

GTRTN:
    if (gti >= MAXTOK){
        gt[MAXTOK-1] = 0;
        fprintf(stderr,"Token too big: %s\n\n",gt);
        exit(999);
    }
    gt[gti] = 0;                /* terminate token */
    if (gtstart == 0){          /* starting column 1? */
        if (c == ','){
            p2err("Invalid use of comma after label, treated as ':'.");
        }
        if (passnr == 2 && c == 0){
            outcom(line);               /* label only */
        }
        c = ':';                /* force to look like label */
    }
    if (c == ' '){              /* eat trailing spaces */
        for (++gtptr; line[gtptr] == ' ' || line[gtptr] == 9; ++gtptr)
           {;}
        c = line[gtptr];
        if (c == ';'){
            gtptr = 200;
            c = 0;
        }else if (c == 0){
            gtptr = 200;
        }else if (c == ',' || c == ':'){
            c = c;
        }else{
            c = ' ';            /* space by default */
            --gtptr;            /* reset line pointer */
        }
    }
    *gtd = c;                   /* return delimiter */
    oldd = c;                   /* remember delimiter */
    ++gtoks;
    ++gtptr;                    /* skip delimiter */
    return TRUE;
}

/* ===============================>>> newlin <<<==============================*/
newlin(void)

{
    short int i;

    if (expanding){
        if (getmline(line)){            /* get a line from macro expansion */
            goto NL1;
        }
    }
    alineno++;
    if (incflag){                       /* if including from COPY file */
        if (!fgets(line,179,chinc)){    /* get line from include file */
            incflag = FALSE;            /* end of COPY file */
            fclose (chinc);             /* close it */
        }
    }

    if (!incflag){                      /* if not including */
        if (!fgets(line,179,chin)){     /* get line from input file */
            printf("\nError: Missing END directive - Aborting assembly!\n");
            printf("\nEnd of Pass 1 -- Last location used: >%x \n",loc);
            exit(0);
        }
    }

NL1:
    i = strlen(line);
    if (line[i-1] == '\n'){
       line[i-1] = 0;                   /* wipe out newline */
    }
    return TRUE;
}

/* ===============================>>> OUTCOM <<<==============================*/
outcom(otcm)

char *otcm;
{
    char strout[5];
    int  colcnt;

    if (expanding){                     /* decide if want to return */
        if (listmacros == MNEVER){      /* return if never */
            return TRUE;
        }
        if (listmacros == MONCE){
            if (!macfirst){
                return TRUE;
	    }                           /* return if not first time */
        }
    }
    chkpage();
    if (listst && liston ){
        sprintf (strout, "%4d", alineno);
        emtsr(strout);
        emtsr("                 ");
        colcnt = 0;            /* listings start at col 21 */
        strout[0] = ' ';
        strout[1] = 0;
        for (; *otcm; ++otcm){
            if ( *otcm == 9 ){

danother:
                emtsr(strout);
                colcnt++;
                if ((colcnt/8) * 8 != colcnt){
                    goto danother;
                }
            }else{
                fputc(*otcm,chlst);
                colcnt++;
            }
        }
        emtsr("\n");
    }
}

/* ===============================>>>chkpage<<<==============================*/
void chkpage(void)

{
    char strout[5];

    if (++lineno > MAXLINE){
        lineno = 2;
        pageno++;
        emtsr("\f");
        emtsr("9640 and 99/4A Assembler\n");
        emtsr(titl);
        emtsr("  Page ");
        if (listst && liston ){
            sprintf (strout, "%4d", pageno);
	}
        emtsr(strout);
        emtsr("\n");
    }
}

/* ===============================>>> lookup <<<============================ */
lookup(losym, loval)

char *losym;
long *loval;
{
    /* look up symbol, return value of first occurrence, TRUE if found */

    int lsin,lsyt,hash,savehash,fndflag;

    *loval = 0;             /* 1-17-88 split *loval = lsin = 0 */
    lsin = 0;
    lsyt = 1;              /* start table at 1 so can return 0 if not found */

    /* compute the hash value for this symbol */
    hash = 0;
    while (losym[lsin] != 0){
      hash = hash * 1235 + cupper(losym[lsin++]);
    }

    if (hash < 0){
        hash = -hash;
    }
    hash = (hash % MAXVAL) + 1;
    savehash = hash;

    /* scan through table until found or table exhausted */

    while (TRUE){                   /* symbol table search */
        while (TRUE){               /* symbol entry compare */
            lsyt = symind[hash];
            if (!lsyt){             /* is there a symbol here? */
                return FALSE;
    	    }else{                  /* no, symbol not found */
            lsin = 0;
            fndflag = TRUE;
            while (fndflag){        /* symbol compare */
                if (cupper(losym[lsin]) == symt[lsyt++]){
                    if (losym[lsin++] == 0){
                        *loval = symval[hash];
                        return hash;/* return index into table */
                    }
                }else{
                fndflag = FALSE;    /* this isn't the symbol */
                }
            }
            }
            hash++;                 /* else, bump hash pointer */
            if (hash > MAXVAL){     /* handle circular wrap around */
                hash = 1;
	    }
            if (hash == savehash){
                return FALSE;       /* table completely full, and not there */
            }
        }
    }                               /* end symbol table search */
}                                   /* end lookup */
