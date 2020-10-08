/*                   994a and GENEVE 9640 loader

   This program is used to load a set of ti-99/4a object files into
   a psuedo RAM image (64k).  It maps in REF/DEF statements.  After the 
   successful load, it creates one or more program image files according
   to the program lists.

   Calling Sequence:

      loader [>list_file] load_control_file

   The loader control file consists of link commands in the following
   form:

      OPTION   G/F/N[,modstar]   (slow geneve, fast geneve, ti-99 mode)
      INCLUDE  filename          (included object file)
      PAGEBOUND                  (put on >100 page boundry)
      SEGBOUND                   (put on >800 segment boundry)
      RELATIVE #                 (relative offset remaining file)
      SAVE     filename,start,end[,modsize]
      END

   Comment Lines have exclamation mark.

   COPYRIGHT 1987,1990 BY ALAN L. BEARD

   Update History:

   1.0  30-November-1987  First Release into public domain.  Released
			  to BIX.

   1.1  09-December-1987  Correct multiple module generation.

   1.2  27-December-1987  Add optional argument to SAVE statement,
			  which specifies the largest module to be
			  saved.

   1.3  02-January-1988   Changes with Ron Lepine for TIPRO compatibility

   1.4  28-January-1988   Add in relocatable code, since a99 can now
			  generate it.

   1.5  08-February-1988  More MSDOS compatibility.  Add PAGEBOUND &
                          SEGBOUND commands.

   1.6  09-May-1988       Added some more debug stuff on error printout

   1.7  19-MAR-1989       Fixed OPTION command to correctly pick up
                          the start address to load                     

   2.9  31-May-1989       Version Number changed to match a99, install
                          auto-detection of compressed object code.
                          Removed debug statements.  Changed object read
                          to read only 80 bytes for an object record.

   2.10 23-Jun-1990       Allow relative and absolute object to be mixed
                          (to allow c99 PRINTF routine).   Also handle
                          problem with REF's with no ref chain (ref that
                          wasn't used).

   2.11 24-Jul-1990       Allow new REF's/DEF's in table of up to 31 chars
                          instead of just 6 chars.

   2.12 27-Aug-1990       Add comment line capability (via exclamation
                          mark).   Change "memory" to 65535 (was 65536)
                          so that it can be compiled w/MicroSoft C.  Make
                          "far" reference "_far".

   2.13 29-dec-1990       Add "relative" to build segmented executables
                          for geneve, including new "X" opcode.
*/

#include <stdio.h>
#ifdef MSDOS
#include <memory.h>
#endif

#define VERSION  "TI-99/4A & 9640 Loader Version 2.13 (29dec90)"
#define OPENRD "r"		/* open for read access */
#define OPENWT "wb"		/* open for write access */
#define TRUE 1			/* something is true def */
#define FALSE 0			/* something is false def */
#define MAXREFS 800		/* Maximum REF's total */
#define MAXLINE 82		/* Maximum size of input line */
		
unsigned short int pcptr,	/* program counter */
         start,			/* start address of module */
         endadr,		/* end address of module */
         size,			/* module size */
         modstar,		/* module start address */
         maxmodu,		/* maximum module size */
         relflag,		/* found relative object code */
         compobj,               /* processing compressed object (1) or not */
	 reloff,                /* relative offset */
         idtval;                /* ident value */

  short int comptr,		/* current command pointer */
        refptr,                 /* ref/def list pointer */
        chksum,			/* record checksum */
        lineno,                 /* current record processing object line */
        linesiz;		/* size of current input line */

#ifdef MSDOS
  unsigned char _far memory[65535];  	/* TI-99 64k memory space */
#else
  unsigned char memory[65535];		/* TI-99 64k memory space */
#endif

  char  option;			/* type of load module */

  char	refs[31*MAXREFS],	/* ref list */
        reft[MAXREFS],          /* ref type (R for ref, D for def) */
	line[MAXLINE+1],	/* input buffer */
        idt[32],                /* ident */
        name[10],		/* program name */
        inputf[80],		/* command input file name */
        xrec[128],		/* an XMODEM record */
        incfil[80];		/* include file name */

   int	refval[MAXREFS];	/* refs */


  FILE *chin, *fopen(), *chic, *chout;   /* file pointers */

      
/* =============================>>> MAIN  <<<============================= */
  main(argc,argv)
  int argc;
  char *argv[];
  {
    short int unres, i, k;
    unsigned short int j;
    long int  m;

    printf("\n%s\n",VERSION);
    printf("Copyright 1990 by LGMA Products\n");

    if ( argc != 2 )
      {
      printf ( "Wrong Number of Arguments, (%d) s/b (1)\n",argc);
      printf ( "\nUsage; loader command_filename\n");
      exit();
      }
    strcpy ( inputf, argv[1] );

/*  Input arguments successfully scanned.  Now open the input file */

      if (!(chin = fopen(inputf,OPENRD)) )
          {
          printf("Fatal Error: Can't open input file %s, error %d\n",
             inputf, chin);
          exit(999);
          }

/*  The input and listing files have been successfully opened.  Now
    clear the resulting object file */

#if MSDOS
      _fmemset(&memory,0,65536);	/* clear TI-99 address space */
#else
      setmem(&memory,65536,0);		/* clear TI-99 address space */
#endif

/*  Now that memory is cleared, do for each line in the input file 
    (scan the line).    */

      refptr = -1;		/* pointer to ref/def list */
      option = 0;		/* type of load module */
      relflag = FALSE;		/* found relative object code or not */
      reloff = 0;		/* relative offset */

      while ( newlin() && procrec() )
                 ;
/*  List the resulting Def symbol file.  Count the number of unresolved
    refs     */

      printf ("Symbol File (DEF entries)\n");
      printf (" Total Entries = %d\n", refptr + 1 );

      unres = 0;			/* unresolved REF counter */  
      for ( i=0; i<=refptr; i++ )
        {
	if ( reft[i] == 'R')		/* if unresolved reference */
          unres++;			/* bump counter */

	else {
          for ( k=0; k<31; k++ )
 	    idt[k] = refs[(i*31)+k];
          idt[31] = 0;
          j      = refval[i];
/* following printf split due to problems with Lattice -C- under TIPRO */
	  printf(" %4d ",i+1);
	  printf(" %s ",idt);
	  printf(" %c ",reft[i]);
	  printf(" %x\n",refval[i]);
	  }
        }

/* Check for any unresolved references.  If so, then display them */

       if ( !unres )
         printf(" NO Unresolved references\n");

       else {
/*  List the resulting unresolved references */

        printf ("Symbol File Unresolved References:\n");
        for ( i=0; i<=refptr; i++ )
          {
          if ( reft[i] == 'R' ) 	/* if unresolved reference */
	    {
            for ( k=0; k<31; k++ )
 	      idt[k] = refs[(i*31)+k];
            idt[31] = 0;
            j      = refval[i];
/* following printf split due to problems with Lattice -C- under TIPRO */
	    printf(" %4d ",i+1);
	    printf(" %s ",idt);
	    printf(" %c ",reft[i]);
	    printf(" %x\n",refval[i]);
            }
          }      
          printf("Number of unresolved references = %d\n",unres );
       }
/*  Processing done.  Make sure all files are closed.  Ignore errors */

    fclose ( chout );			/* output file */
    fclose ( chin  );			/* input file */
    fclose ( chic  );			/* include file */
  }

/* ============================ procrec ============================ */

procrec()
{
/* procrec - process a linker command record, record is in string
             "line".   */
   short int incptr, fndptr;

   printf ("> %s\n",line);

   comptr = 0;				/* command pointer */
   compobj = FALSE;			/* not using compressed object */

/*  Empty command, just skip it */

   if ( !skipbl() )
     return(TRUE);
					/* END command, just return */
   if ( nextch("END") )
     return FALSE;

   if ( nextch("OPTION") )		/* OPTION command */
     {
     skipbl();
     if ( nextch("F") )	{		/* OPTION = Geneve FAST Mode */
       pcptr  = 0x0400;			/* initial start load address */
       option = 'F';
       }
     else if ( nextch("G") ) {		/* OPTION = Geneve SLOW Mode */
       pcptr = 0x0400;
       option = 'G';
       }
     else if ( nextch("N") ) {		/* OPTION = TI-99/4A Mode */
       pcptr = 0xA000;
       option = 0;
     } else {
       printf("Bad OPTION specified, not G, F, or N\n");
       return FALSE;
       }

     if ( line[comptr] != 0 ) {		/* if not at EOL */
       skipbl();
       if ( nextch(",") ) {		/* next character a comma? */
	 comptr++;
	 skipbl();
         gethex();
	 pcptr = idtval;		/* start address to load */
	 }
       }
     return TRUE;
     }

   if ( nextch("RELATIVE") ) {		 /* RELATIVE command */
     skipbl();
     if ( line[comptr] == '>' )
       comptr++;			 /* skip useless ">" */
     gethex();
     reloff = idtval;
     printf("New Relative Offset is %x\n",reloff);
     return TRUE;
   }

   if ( nextch("SAVE") )  {              /* SAVE command */
     skipbl();
     incptr = 0;
     while ( line[comptr] !=0 && line[comptr] != '\n' &&
             line[comptr] !=' ' && line[comptr] != ',' )
       incfil[incptr++] = line[comptr++];

     incfil[incptr++] = 0;

/*  Extract start and end addresses.  These may be specified as
    symbols or as a hex number starting with a > sign */

     if ( line[comptr++] != ',' ) {
       printf ( "Error - Bad Start SAVE Address\n");
       return FALSE;
       }

     if ( line[comptr] == '>' ) {
       comptr++;
       gethex();
       start = idtval;
     } else {
       getidt();
       fndptr = fndidt();
       if ( fndptr < 0 ) {
         printf ( "Error - Cannot find start symbol %s\n",idt);
         return FALSE;
         }
       start = refval[fndptr];
       }

/* extract end address */

     if ( line[comptr++] != ',' ) {
       printf ( "Error - Bad End Address\n");
       return FALSE;
       }

     if ( line[comptr] == '>' ) {
       comptr++;
       gethex();
       endadr = idtval;

     } else {
       getidt();
       fndptr = fndidt();
       if ( fndptr < 0 ) {
         printf ( "Error - Cannot find start symbol %s\n",idt);
         return FALSE;
         }
       endadr = refval[fndptr];
       }

/* extract optional argument, maximum program module save size */

     if ( line[comptr++] != ',' ) 
       maxmodu = 0x2000;

     else if ( line[comptr] == '>' ) {
       comptr++;
       gethex();
       maxmodu = idtval;
     } else {
       getidt();
       fndptr = fndidt();
       if ( fndptr < 0 ) {
         printf ( "Error - Cannot find max symbol %s\n",idt);
         return FALSE;
         }
       maxmodu = refval[fndptr];
       }
       savfil();			/* save the files */
       return TRUE;
     }


   if ( nextch("INCLUDE") ) {		/* INCLUDE command, extract file */
     skipbl();
     incptr = 0;
     while ( line[comptr] != 0 && line[comptr] != '\n' &&
             line[comptr] != ' ')
       incfil[incptr++] = line[comptr++];

     incfil[incptr++] = 0;

/*  Open the include file, binary input, file name incfil */

      if (!(chic = fopen(incfil,OPENRD)) ) {
          printf("Fatal Error: Can't open include file %s, error %d\n",
             incfil, chic);
          exit(999);
          }
      procfil ();
      fclose ( chic );			/* close include file */
      return TRUE;
     }

/* Page Boundry (round to next >100) Command */

   if ( nextch("PAGEBOUND") ) {		/* PAGEBOUND command */
     printf("program counter before rounding %x\n",pcptr);
     pcptr = ( pcptr + 255 ) & 0xff00;	/* round up to next page */
     return TRUE;			/* good return */
     }

/* Segment Boundry (round to next >800) Command */
   if ( nextch("SEGBOUND") ) {		/* SEGBOUND command */
     printf("program counter before rounding %x\n",pcptr);
     pcptr = ( pcptr + 0x7ff ) & 0xf800;
     return TRUE;
     }

/* No valid command found.  Abort! */

  printf("\nError - Unrecognized Command, Aborting loader!\n");
  exit();
}
/* ===============================>>> procfil <<<============================*/
procfil()
{
/*  procfil - process a single object file.  */

  if (!newobj()) {
    printf("\nError - Object File %s is Empty\n",incfil);
    exit();
    }

  if (line[0] == '0' ){
    compobj = FALSE;		/* uncompressed object code */

  }else{
    if (line[0] == 01 ){
      compobj = TRUE;		/* compressed object code */

    }else{
      printf("\nError - Bad Header on Object File %s\n",incfil);
      exit();
      }
  }

  chksum = 0;			/* checksum on record */
  comptr = 0;			/* command line pointer */
  lineno = 1;			/* line number in object line */
  while ( proctok() )
   ;
}
 
/* ===============================>>> proctok <<<============================*/

proctok()
{
/* proctok - process a token in the input command line, comptr points to
   command */

unsigned short int nloc,loc;
short int i,fndptr;
char mytok;

  mytok = line[comptr++];

  switch ( mytok ) {

/* case 0, 8 character program ident, 4 hex digit length */
    case '0':
    case 1:
      gethex();			/* get hex value */
      size = idtval;
      for ( i=0; i<8; i++ )
        name[i] = line[comptr++];

      name[9] = 0;
      modstar = pcptr;		/* start load address */
      printf ("Processing module %s, start %x, size %x\n",name,modstar,size);
      break;

/* case 3, External Relative REF                      */
/* case 4, External Absolute REF                      */
    case '3':
    case '4':
      gethex();			/* get hex value into idtval */
      if ( mytok == '3' ) {
        idtval += modstar;	/* do relative bump */
        relflag = TRUE;		/* set found relative */
	}

      else {
        relflag = FALSE;	/* set found absolute */
	}

      getidt();			/* get an ident into idt */
      fndptr = fndidt();	/* search ident in ref/def table */

      if ( fndptr != -1 ) {

/* Symbol already in table.  If symbol is a DEF, then execute REF
   chain.  If symbol is a ref, then we have a double REF, and this
   chain must be searched to logical end, the ending terminator
   0 must be replaced by the current symbol value, and the current
   symbol value must be replaced by the new chain start */

/* printf("found REF %s, reft[fndptr] %d\n", idt, reft[fndptr] ); */

	if ( reft[fndptr] == 'D' ) { 	/* DEF in table, execute chain */
	  loc  =  idtval;		/* location to start chain */
	  nloc =  loc;
	  idtval = refval[fndptr];	/* value to store for DEF */
	  if ( loc != 0 ) {
	    do {
              nloc = (memory[loc]<<8) + memory[loc+1];
	      memory[loc]   =  idtval >> 8;
	      memory[loc+1] =  idtval & 0xff;
/* printf("memory chain loc %x, nloc %x\n",loc, nloc); */
	      loc = nloc;
	      } while ( loc );
            }
        } else {			/* double REF across modules */
	  loc = refval[fndptr];		/* current REF chain start */
	  nloc = loc;
	  if ( loc != 0 ) {
	    do {
	      nloc = loc;
              loc = (memory[loc] << 8) + memory[loc+1];
/* printf("2nd memory chain loc %x, nloc %x\n",loc, nloc); */
	      } while ( loc );
	    }

/* Exit from above, nloc is the ending location of the current REF'd
   chain.  Set that memory location to the start of the new REF
   chain */
	  if ( nloc != 0 ) {
	    memory[nloc]   = idtval >> 8;
	    memory[nloc+1] = idtval & 0xff;
	    }
          }
        }
/*				   REF not in table, start an entry */
      else
	entsym ( 'R' );			/* enter as a REF */
      break;

/* case 5, External Relative DEF */
/* case 6, External Absolute DEF */
    case '5':
    case '6':
      gethex();				/* get the location */
      getidt();
      if ( mytok == '5' )		/* is it relative? */
        {
	idtval += modstar;
	relflag = TRUE;
	}

      fndptr = fndidt();		/* see if already in ref/def table */
      if ( fndptr < 0 ) 		/* not in ref/def table */
	entsym('D');			/* enter into symbol table */	  
      else {
	if ( reft[fndptr] == 'D' ) {	/* if another DEF */
	  printf("Error - Duplicate DEF %s at locations %x, %x\n",
              idt, refval[fndptr], idtval );
	} else {		        /* code here for tracing REF chain */
	  reft[fndptr] = 'D';		/* change REF entry into DEF */
	  loc  =  refval[fndptr];	/* location to start chain */
	  refval[fndptr] = idtval;	/* new value for REF */
	  do {
            nloc = (memory[loc]<<8) + memory[loc+1];
	    memory[loc]   =  idtval >> 8;
	    memory[loc+1] =  idtval & 0xff;
	    loc = nloc;
            } while ( loc );
	  }
        }
      break;

/* case 7, 4 hex digit checksum (ignored for now) */
    case '7':
      gethex();				/* skip any other for now */
      break;

/* case 8, 4 hex digits ignore */
    case '8':
      gethex();				/* skip checksum ignore */
      break;

/* case 9, Absolute LOAD Address */
    case '9':
      gethex();
      pcptr = idtval;
      if ( relflag ) {
        printf("Error - Mixed Absolute/Relative Object, type 9\n");
        printf(" on line #%d, char #%d, value #%d\n",lineno,comptr,mytok);
        exit(999);
        }
      break;

/* case A, Relative LOAD Address */
    case 'A':
      gethex();
      pcptr = idtval + modstar;
      relflag = TRUE;
      break;

/* case B, 4 hex digits absolute data */
    case 'B':
      gethex();
      memory[pcptr++] = (idtval>>8) & 0xff;
      memory[pcptr++] = idtval & 0xff;
      break;

/* case C, 4 hex digits of relative data */
    case 'C':
      gethex();
      idtval += modstar;		/* relative process */
      if ( reloff != 0 ) {		/* real kludge for bl *r12 */
        if ( !(memory[pcptr-2] == 0x06 && memory[pcptr-1] == 0x9c) )
           idtval = idtval - reloff;		/* - relative offset */
      }
      memory[pcptr++] = (idtval>>8) & 0xff;
      memory[pcptr++] = idtval & 0xff;
      break;

/* case X, 4 hex digits of true (non offsetted) relative data */
    case 'X':
      gethex();
      idtval += modstar;		/* relative process */
      memory[pcptr++] = (idtval>>8) & 0xff;
      memory[pcptr++] = idtval & 0xff;
      break;

/* case F, end of record */
    case 'F':
      comptr = 0;			/* reset command pointer */
      lineno++;				/* bump line number */
      if ( !newobj() ) {		/* get an object record */
        printf("\nError, premature EOR on object file\n");
        printf(" on line #%d, char #%d, value #%d\n",lineno,comptr,mytok);
        exit(999);
        }
      break;

/* case :, end of file */
    case ':':
      pcptr = modstar + size;		/* bump module start */
      return FALSE;
      break;

/* else, bad object module */
    default:
      printf("\nError - Bad Object Module\n");
      printf(" on line #%d, char #%d, value #%d\n",lineno,comptr,mytok);
      exit(999);
      }
    return TRUE;
}
/* ===============================>>> newlin <<<==============================*/
  newlin()
  {
    if (!fgets(line,MAXLINE,chin))
      {
       printf("\nError: Missing END directive - Aborting loader!\n");
       exit();
      }

    linesiz = strlen(line);
    if (line[linesiz-1] == '\n')
       line[linesiz-1] = 0;		/* wipe out newline */
    return TRUE;
  }

/* ===============================>>> newobj <<<==============================*/
  newobj()
  {
  short int i;

    for ( i=0; i<80; i++ ){
        line[i] = fgetc(chic);
        }
    if ( line[79] == EOF && line[1] != ':' ){
        return FALSE;
    }
    line[80] = 'F';			/* terminate line */
    return TRUE;
  }

/* =========================== skipbl ============================= */

skipbl()
{
/* This function skips blanks.  If the EOL or comment is encountered, then
   the function returns FALSE, otherwise TRUE */

  while ( line[comptr] == ' ' )
    comptr++;

  if ( line[comptr] == '\0' || line[comptr] == '!' )
    return FALSE;

  return TRUE;

}

/* ========================= nextch ============================= */

nextch(t)           /* parse next characters in command line */
char t[];
{
short int savptr,i;

  i = 0;                        /* initial line scan input */
  savptr = comptr;		/* so I can fail back */

  while ( (t[i] != 0) && ((t[i] == line[comptr++])) )
     i++;

  if ( t[i] == 0 )
    return TRUE;

  comptr = savptr;
  return FALSE;

}

/* ============================>>> gethex <<<==========================*/
gethex()
{
/* gethex - translates the next four ascii characters into a single
            value.  Characters are in line[comptr].  Value is idtval.

            gethex honors the current compobj (compressed object) flag
            and extracts ascii if FALSE, extracts two hex bytes if TRUE */

short int idtcha, i;

  if ( compobj ){
    idtval = ( line[comptr++] * 256 );
    idtval = idtval + ( line[comptr++] & 0xff );
  }else{
    idtval = 0;
    for ( i=0; i<4; i++ ) {
      idtval *= 16;
      idtcha = line[comptr];
      if ( idtcha >= '0' && idtcha <= '9') {
        idtval += (idtcha - 48) ;
        comptr++;
      } else {
        if ( idtcha >= 'A' && idtcha <='F' ) {
          idtval += (idtcha - 55);
          comptr++;
          }
        }
      }
    }
  return TRUE;
}

/* ============================>> getidt <<<============================= */
getidt()
{
/*  getidt - get the next n character ident from line into idt */
short int i;
unsigned char symlen;

  for ( i=0; i<31; i++ )
    idt[i] = ' ';

  symlen=line[comptr];
/*  check for long ident, the first character is 7 to 31 and represents
    the number of characters which follow                              */
  if ( symlen > 6 && symlen < 32 ) {
    comptr++;
    for ( i=0; i<symlen; i++ )
      idt[i] = line[comptr++];
  } else {
    for ( i=0; i<6; i++ ) {
      if ( (line[comptr] != ',') && (line[comptr] != 0) 
                                 && (line[comptr] != '\n') ) 
        idt[i] = line[comptr++];
      else
        idt[i] = ' ';
      }

    idt[31] = 0;
   }
  return TRUE;
}

/* ==========================> fndidt <============================== */
fndidt ()
{
/* fndidt - searches the ref/def list for the ident in idt.  If matched,
            return index.  If not matched, return -1 */

short int idtp, refp, badsym;
int fndsym;

  fndsym = -1;

  for ( idtp=0; (idtp<=refptr) && (fndsym == -1) ; idtp++ ) {
    badsym = FALSE;
    for ( refp=0; refp<31 && !badsym; refp++ ) {
      if ( idt[refp] != refs[31*idtp+refp] )
        badsym = TRUE;
      }

    if ( !badsym ) 
      fndsym = idtp;
    }

    return fndsym;
}      

/* ==========================> ENTSYM <============================== */
entsym ( enttyp )

char enttyp;
{
/*  entsym: enters the symbol in idt with a value of idtval into the
	    refdef table.  The symbol type 'R' or 'D' is passed as
	    enttyp */
short int i;

  refptr++;
  if ( refptr > MAXREFS ) {
    printf("Exceeded Maximum Number of References, >%d\n",MAXREFS);
    exit(999);
    }

  refval[refptr] = idtval;

  for ( i=0; i<31; i++ )
    refs[(refptr*31)+i] = idt[i];

  reft[refptr] = enttyp;
  return TRUE;
}
/* =============================>>  savfil  <<=============================*/

savfil()
{
/*  savfil - This module saves the output files for the SAVE command */

  unsigned short int totsiz, memval, mymax, mymodus;
  short int nomodu, nnmodu, i, j, thisfil, ouerr, lastrec;

/*  Calculate the sizings for the output file(s).  >2000 bytes per
    file */

   mymax  = maxmodu - 6;		/* maximum mod size discounting head */
   totsiz = endadr - start;		/* size of output file(s) */
   nomodu = (totsiz / mymax) + 1; 	/* number of modules */

   if ( nomodu > 6 ) {			/* error check, limit # modules */
     printf ("Error-Too many modules calculated, %d\n",nomodu);
     exit(999);
     }

   for ( i=0; i<nomodu; i++ ) {		/* do for each module */

/*  Open the output file, binary output, file name incfil */
    printf("Creating OUTPUT file %s\n",incfil);

     if (!(chout = fopen(incfil,OPENWT)) ) {
       printf("Fatal Error: Can't open output file %s, error %d\n",
         incfil, chout);
       exit(999);
       }
/*  The following creates a TIFILES header.  TIFILES header is the
    TI-99 XMODEM format popularized by Paul Charlton.  It is a single
    128 byte xmodem record which precedes the remainder of the 
    downloaded file, and tells the XMODEM program how to store the
    file (e.g. type=PROGRAM, length, etc.).                         */

     xrec[0] = 0x07;                   /* ti files header */
     xrec[1] = 'T';
     xrec[2] = 'I';
     xrec[3] = 'F';
     xrec[4] = 'I';
     xrec[5] = 'L';
     xrec[6] = 'E';
     xrec[7] = 'S';

     if ( totsiz > mymax ) {		/* if this not last module */
       mymodus = mymax + 255;		/* total size rounded up nearest sect */
       xrec[8]  = 0;			/* # of sectors in file */
       xrec[9]  = mymodus >> 8;		/* my number of sectors */
       xrec[10] = 0x01;			/* type of file (program) */
       xrec[11] = 0;			/* # of records/sector */
       xrec[12] = 0;			/* # bytes in last sector */
       xrec[13] = 0;			/* # of records in file */
       xrec[14] = 0;
       thisfil  = mymax;		/* # bytes to write this time */
       totsiz = totsiz - mymax;		/* total size decrement */
       lastrec = 0;			/* # bytes to pad last record */
     }else{
       mymodus = totsiz + 6 + 256;	/* this module's size */
       xrec[8] = 0;			/* total # sectors in file */
       xrec[9] =  mymodus >> 8;
       xrec[10] = 0x01;			/* type of file (program) */
       xrec[11] = 0;			/* # of records/sector */
       xrec[12] = mymodus & 0xff;	/* # bytes last sector */
       xrec[13] = 0;
       xrec[14] = 0;			/* # of records in file */
       thisfil  = totsiz;
       if ( xrec[12] )			/* if last sector full */
	 lastrec = 0;			/* don't pad it */
       else				/* otherwise, */
         lastrec  = 256 - xrec[12];	/* # bytes to pad last record */
     }
     for ( j=15; j<128; j++ )
       xrec[j] = ' ';			/* clear last part of record */

/* write out the first 128 bytes, which is the TIFILES header */
     for ( j=0; j<128; j++ ) {
       ouerr = fputc ( xrec[j], chout );
       if ( ouerr == EOF ) {
         printf("Error writing to output file, %d\n",ouerr);
         exit(999);
         }
       }
/* Now write out this module.  Format the first 6 bytes.  These are:

         FFFF/0 - If more files follow/If last file
         mmmm - The number of bytes, including header
         nnnn - The start address to load                */
          
     if ( i == (nomodu-1) ) {             	/* Last Module */
       xrec[0] = 0x00;
       xrec[1] = option;
       nnmodu  = totsiz + 6;
       xrec[2] = nnmodu >> 8;
       xrec[3] = nnmodu & 0xff;
       xrec[4] = start >> 8;
       xrec[5] = start & 0xff;
     }else{				      /* Not the Last Module */
       xrec[0] = 0xff;
       xrec[1] = option;
       xrec[2] = mymax >> 8;
       xrec[3] = mymax & 0xff;
       xrec[4] = start >> 8;
       xrec[5] = start & 0xff;
       }

/* write out the first 6 bytes, which is the E/A loader header */
     for ( j=0; j<6; j++ ) {
       ouerr = fputc ( xrec[j], chout );
       if ( ouerr == EOF ) {
         printf("Error writing to output file, %d\n",ouerr);
         exit(999);
         }
       }

/* now write out the actual data, whew */

     for ( j=0; j<thisfil; j++ ) {
       memval = memory[start];
       start++;
       ouerr = fputc ( memval, chout );
       if ( ouerr == EOF ) {
         printf ("Error writing to output file, %d\n",ouerr);
         exit(999);
         }
       }

/* pad last sector with zeroes (round up to even 256 byte sectors */
     for ( j=0; j<lastrec; j++ ) {
       ouerr = fputc ( 0, chout );
       if ( ouerr == EOF ) {
         printf ("Error writing to output file, %d\n",ouerr);
         exit(999);
         }
       }
	
     fclose ( chout );		/* close the output file */

/* increment the last character of the file name in case another file
   is needed.  Prescan the file name to see if a period (.) is present.
   If so, then bump the previous character.  If not, bump the last
   character */

     j=0;
     while ( incfil[j] != 0 && incfil[j] != '.' )
       j++;

     j--;				/* point to previous char */
     incfil[j]++;
   }
}
