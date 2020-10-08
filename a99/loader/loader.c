/*            99/4a loader

   This program is used to load a set of ti-99/4a object files into
   a psuedo RAM image (64k).  It maps in REF/DEF statements.
   After the successful load, it creates one or more program
   image files according to the program lists.

   Calling Sequence:

      loader -l(list file) (load control file)

   The loader control file consists of link commands in the following
   form:

      OPTION   G/F/N[,modstar]   (slow geneve, fast geneve, ti-99 mode)
      INCLUDE  filename          (included object file)
      PAGEBOUND
      SEGBOUND
      SAVE     filename,start,end[,modsize]
      END

   COPYRIGHT 1987,1988 BY ALAN L. BEARD

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
                          the start address to load                     */
#include <stdio.h>

#define VERSION  "TI-99/4A & 9640 Loader Version 1.7 (19-MAR-89)"
#define OPENRD "r"		/* open for read access */
#define OPENWT "wb"		/* open for write access */
#define TRUE 1			/* something is true def */
#define FALSE 0			/* something is false def */
#define MAXREFS 800		/* Maximum REF's total */
#define MAXLINE 82		/* Maximum size of input line */
		
unsigned short int pcptr,	/* program counter */
       start,			/* start address of module */
       endadr,			/* end address of module */
       size,			/* module size */
       modstar,			/* module start address */
       maxmodu,			/* maximum module size */
       relflag,			/* found relative object code */
       idtval;                  /* ident value */

  short int comptr,		/* current command pointer */
       refptr,                  /* ref/def list pointer */
       chksum,			/* record checksum */
       linesiz;			/* size of current input line */

#if MSDOS
  unsigned char far memory[65536];  	/* TI-99 64k memory space */
#else
  unsigned char memory[65536];		/* TI-99 64k memory space */
#endif

  char  option;			/* type of load module */

  char	refs[6*MAXREFS],	/* ref list */
        reft[MAXREFS],          /* ref type (R for ref, D for def) */
	line[MAXLINE+1],	/* input buffer */
        idt[7],                 /* ident */
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

    printf("\n%s\n",VERSION);
    printf("Copyright 1987,1988,1989 by Al Beard/Ron Lepine\n");

    if ( argc != 2 )
      {
      printf ( "Wrong Number of Arguments, (%d) s/b (1)\n",argc);
      printf ( "\nUsage; loader command_filename\n");
      exit();
      }
    strcpy ( inputf, argv[1] );

#ifdef DEBUG
    printf ("DONE arg parse, inp file %s\n",inputf );
#endif

/*  Input arguments successfully scanned.  Now open the input file */

      if (!(chin = fopen(inputf,OPENRD)) )
          {
          printf("Fatal Error: Can't open input file %s, error %d\n",
             inputf, chin);
          exit(999);
          }

/*  The input and listing files have been successfully opened.  Now
    clear the resulting object file */

    setmem(&memory,65536,0);	/* clear TI-99 address space */

/*  Now that memory is cleared, do for each line in the input file 
    (scan the line).    */

      refptr = -1;		/* pointer to ref/def list */
      option = 0;		/* type of load module */
      relflag = FALSE;		/* found relative object code or not */

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

	else
          {
          for ( k=0; k<6; k++ )
 	    idt[k] = refs[(i*6)+k];
          idt[6] = 0;
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

       else
         {
/*  List the resulting unresolved references */

        printf ("Symbol File Unresolved References:\n");
        for ( i=0; i<=refptr; i++ )
          {
          if ( reft[i] == 'R' ) 	/* if unresolved reference */
	    {
            for ( k=0; k<6; k++ )
 	      idt[k] = refs[(i*6)+k];
            idt[6] = 0;
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

   comptr = 0;

/*  Empty command, just skip it */

   if ( !skipbl() )
     return(TRUE);
					/* END command, just return */
   if ( nextch("END") )
     return FALSE;

   if ( nextch("OPTION") )		/* OPTION command */
     {
     skipbl();
     if ( nextch("F") )			/* OPTION = Geneve FAST Mode */
       {
       pcptr  = 0x0400;			/* initial start load address */
       option = 'F';
       }
     else if ( nextch("G") )		/* OPTION = Geneve SLOW Mode */
       {
       pcptr = 0x0400;
       option = 'G';
       }
     else if ( nextch("N") )		/* OPTION = TI-99/4A Mode */
       {
       pcptr = 0xA000;
       option = 0;
       }
     else
       {
       printf("Bad OPTION specified, not G, F, or N\n");
       return FALSE;
       }

     if ( line[comptr] != 0 )		/* if not at EOL */
       {
#ifdef DEBUG
       printf(" Parsing hex string, not at eol 1\n");
#endif
       skipbl();
       if ( nextch(",") )		/* next character a comma? */
	 {
#ifdef DEBUG
         printf(" Parsing actual hex value, not at eol 2\n");
#endif
	 comptr++;
	 skipbl();
         gethex();
	 pcptr = idtval;		/* start address to load */
	 }
       }
#ifdef DEBUG
       printf("Returned OPTION %x, pcptr %x\n",option, pcptr);
#endif
     return TRUE;
     }
   if ( nextch("SAVE") )                /* SAVE command */
     {
     skipbl();
     incptr = 0;
     while ( line[comptr] !=0 && line[comptr] != '\n' &&
             line[comptr] !=' ' && line[comptr] != ',' )
       incfil[incptr++] = line[comptr++];

     incfil[incptr++] = 0;

#ifdef DEBUG
     printf("Extracted file name %s\n",incfil);
#endif

/*  Extract start and end addresses.  These may be specified as
    symbols or as a hex number starting with a > sign */

     if ( line[comptr++] != ',' ) 
       {
       printf ( "Error - Bad Start SAVE Address\n");
       return FALSE;
       }

     if ( line[comptr] == '>' ) 
       {
       comptr++;
       gethex();
       start = idtval;

#ifdef DEBUG
       printf("Extracted Start SAVE Address %x\n",start);
#endif

       }
     else
       {
       getidt();
       fndptr = fndidt();
       if ( fndptr < 0 )
         {
         printf ( "Error - Cannot find start symbol %s\n",idt);
         return FALSE;
         }
       start = refval[fndptr];
#ifdef DEBUG
       printf ("PROCREC - Found START symbol %s & value %x\n",idt,start);
#endif
       }

/* extract end address */

     if ( line[comptr++] != ',' ) 
       {
       printf ( "Error - Bad End Address\n");
       return FALSE;
       }

     if ( line[comptr] == '>' ) 
       {
       comptr++;
       gethex();
       endadr = idtval;

#ifdef DEBUG
       printf("Ending address is %x\n",endadr);
#endif

       }
     else
       {
       getidt();
       fndptr = fndidt();
       if ( fndptr < 0 )
         {
         printf ( "Error - Cannot find start symbol %s\n",idt);
         return FALSE;
         }
       endadr = refval[fndptr];
#ifdef DEBUG
       printf("Ending symbol %s & address %x\n",idt,endadr);
#endif
       }
/* extract optional argument, maximum program module save size */

     if ( line[comptr++] != ',' ) 
       maxmodu = 0x2000;

     else if ( line[comptr] == '>' ) 
       {
       comptr++;
       gethex();
       maxmodu = idtval;

#ifdef DEBUG
       printf("Maximum Module Size is %x\n",maxmodu);
#endif

       }
     else
       {
       getidt();
       fndptr = fndidt();
       if ( fndptr < 0 )
         {
         printf ( "Error - Cannot find max symbol %s\n",idt);
         return FALSE;
         }
       maxmodu = refval[fndptr];
#ifdef DEBUG
       printf("Max Mod symbol %s & address %d\n",idt,endadr);
#endif
       }
       savfil();			/* save the files */
       return TRUE;
     }


   if ( nextch("INCLUDE") )		/* INCLUDE command, extract file */
     {
     skipbl();
     incptr = 0;
     while ( line[comptr] != 0 && line[comptr] != '\n' &&
             line[comptr] != ' ')
       incfil[incptr++] = line[comptr++];

     incfil[incptr++] = 0;

/*  Open the include file, binary input, file name incfil */

      if (!(chic = fopen(incfil,OPENRD)) )
          {
          printf("Fatal Error: Can't open include file %s, error %d\n",
             incfil, chic);
          exit(999);
          }
      procfil ();
      fclose ( chic );			/* close include file */
     return TRUE;
     }

/* Page Boundry (round to next >100) Command */

   if ( nextch("PAGEBOUND") )		/* PAGEBOUND command */
     {
     pcptr = ( pcptr + 255 ) & 0xff00;	/* round up to next page */
     return TRUE;			/* good return */
     }

/* Segment Boundry (round to next >800) Command */
   if ( nextch("SEGBOUND") )		/* SEGBOUND command */
     {
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

  if (!newobj())
    {
    printf("\nError - Object File %s is Empty\n",incfil);
    exit();
    }

  if (line[0] != '0' ) 
    {
    printf("\nError - Bad Header on Object File %s\n",incfil);
    exit();
    }

  chksum = 0;			/* checksum on record */
  comptr = 0;			/* command line pointer */
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
      {
      gethex();			/* get hex value */
      size = idtval;
      for ( i=0; i<8; i++ )
        name[i] = line[comptr++];

      name[9] = 0;
      modstar = pcptr;		/* start load address */
      printf ("Processing module %s, start %x, size %x\n",name,modstar,size);
      }
      break;

/* case 3, External Relative REF                      */
/* case 4, External Absolute REF                      */
    case '3':
    case '4':
      {
      gethex();			/* get hex value into idtval */
      if ( mytok == '3' )
        {
        idtval += modstar;	/* do relative bump */
        relflag = TRUE;		/* set found relative */
	}
      else if ( relflag )
        {
        printf("Error - Mixed Absolute/Relative Object, type 3/4\n");
	printf(" on line %s\n",line);
        exit(999);
	}
      getidt();			/* get an ident into idt */
      fndptr = fndidt();	/* search ident in ref/def table */

#ifdef DEBUG
      printf ("PROCTOK, returned FNDIDT, fndptr=%d\n",fndptr);
#endif

      if ( fndptr != -1 )
        {
/* Symbol already in table.  If symbol is a DEF, then execute REF
   chain.  If symbol is a ref, then we have a double REF, and this
   chain must be searched to logical end, the ending terminator
   0 must be replaced by the current symbol value, and the current
   symbol value must be replaced by the new chain start */

	if ( reft[fndptr] == 'D' ) 
          {			/* DEF in table, execute chain */
#ifdef DEBUG
	  printf("REF chain, %c\n",reft[fndptr] );
#endif
	  loc  =  idtval;		/* location to start chain */
	  idtval = refval[fndptr];	/* value to store for DEF */
	  do
            {
            nloc = (memory[loc]<<8) + memory[loc+1];
	    memory[loc]   =  idtval >> 8;
	    memory[loc+1] =  idtval & 0xff;
#ifdef DEBUG
	    printf("REF chain, nloc is %x, mem1 %x, mem2 %x\n",
              nloc, memory[loc], memory[loc+1] );
#endif
	    loc = nloc;
            }
	  while ( loc );
          }
        else				/* double REF across modules */
          {
#ifdef DEBUG
	  printf ("Double REF across modules %s\n",idt);
#endif
	  loc = refval[fndptr];		/* current REF chain start */
	  do
            {
	    nloc = loc;
            loc = (memory[loc] << 8) + memory[loc+1];
#ifdef DEBUG
	    printf("REF chain, nloc is %x, loc is %x\n",
              nloc, loc);
#endif
            }
	  while ( loc );

/* Exit from above, nloc is the ending location of the current REF'd
   chain.  Set that memory location to the start of the new REF
   chain */
	  memory[nloc]   = idtval >> 8;
	  memory[nloc+1] = idtval & 0xff;
          }
        }
/*				   REF not in table, start an entry */
      else
	entsym ( 'R' );			/* enter as a REF */
      }
      break;

/* case 5, External Relative DEF */
/* case 6, External Absolute DEF */
    case '5':
    case '6':
      {
#ifdef DEBUG
      printf ("PROCTOK - processing token %c\n",mytok);
#endif
      gethex();				/* get the location */
      getidt();
      if ( mytok == '5' )		/* is it relative? */
        {
	idtval += modstar;
	relflag = TRUE;
#ifdef DEBUG
        printf ("PROCTOK - Adding Relative Offset, %x\n",idtval);
#endif
	}

      fndptr = fndidt();		/* see if already in ref/def table */
      if ( fndptr < 0 ) 		/* not in ref/def table */
	entsym('D');			/* enter into symbol table */	  
      else
	{
	if ( reft[fndptr] == 'D' )	/* if another DEF */
	  {
	  printf("Error - Duplicate DEF %s at locations %x, %x\n",
              idt, refval[fndptr], idtval );
	  }
	else
	  {        /* code here for tracing REF chain */
#ifdef DEBUG
	  printf("REF chain, %c, loc %x\n",reft[fndptr], refval[fndptr] );
#endif
	  reft[fndptr] = 'D';		/* change REF entry into DEF */
	  loc  =  refval[fndptr];	/* location to start chain */
	  refval[fndptr] = idtval;	/* new value for REF */
	  do
            {
            nloc = (memory[loc]<<8) + memory[loc+1];
	    memory[loc]   =  idtval >> 8;
	    memory[loc+1] =  idtval & 0xff;
#ifdef DEBUG
	    printf("REF chain, nloc is %x, mem1 %x, mem2 %x\n",
              nloc, memory[loc], memory[loc+1] );
#endif
	    loc = nloc;
            }
	  while ( loc );
	  }
        }
      }
      break;

/* case 7, 4 hex digit checksum */
    case '7':
      gethex();				/* skip any other for now */
      break;

/* case 8, 4 hex digits ignore */
    case '8':
      gethex();				/* skip checksum ignore */
      break;

/* case 9, Absolute LOAD Address */
    case '9':
      {
      gethex();
      pcptr = idtval;
      if ( relflag )
        {
        printf("Error - Mixed Absolute/Relative Object, type 9\n");
	printf(" on line %s\n",line);
        exit(999);
        }
      }
      break;

/* case A, Relative LOAD Address */
    case 'A':
      {
      gethex();
      pcptr = idtval + modstar;
      relflag = TRUE;
      }
      break;

/* case B, 4 hex digits absolute data */
    case 'B':
      {
      gethex();
      memory[pcptr++] = (idtval>>8) & 0xff;
      memory[pcptr++] = idtval & 0xff;
#ifdef DEBUG
      if ( pcptr > 0xE00 & pcptr < 0x1000 )
         printf(" 'B', loc %x, mem1 %x, mem2 %x\n",pcptr-2,memory[pcptr-2],
         memory[pcptr-1]);
#endif
      }
      break;

/* case C, 4 hex digits of relative data */
    case 'C':
      {
      gethex();
      idtval += modstar;		/* relative process */
      memory[pcptr++] = (idtval>>8) & 0xff;
      memory[pcptr++] = idtval & 0xff;
#ifdef DEBUG
      if ( pcptr > 0xE00 & pcptr < 0x1000 )
        printf(" 'C', loc %x, mem1 %x, mem2 %x\n",pcptr-2,memory[pcptr-2],
         memory[pcptr-1]);
#endif
      }
      break;

/* case F, end of record */
    case 'F':
      {
      if ( !newobj() )			/* get an object record */
	{
        printf("\nError, premature EOF on object file\n");
        exit(999);
        }
      comptr = 0;
      }
      break;

/* case :, end of file */
    case ':':
      {
      pcptr = modstar + size;		/* bump module start */
      return FALSE;
      }
      break;

/* else, bad object module */
    default:
      {
      printf("\nError - Bad Object Module\n");
      printf(" on line %s\n",line);
      exit(999);
      }
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
       line[linesiz-1] = 0;          /* wipe out newline */
    return TRUE;
  }

/* ===============================>>> newobj <<<==============================*/
  newobj()
  {
    if (!fgets(line,MAXLINE,chic))
      return FALSE;
       
    linesiz = strlen(line);
    if (line[linesiz-1] == '\n')
       line[linesiz-1] = 0;          /* wipe out newline */

    return TRUE;
  }

/* =========================== skipbl ============================= */

skipbl()
{
/* This function skips blanks.  If the EOL is encountered, then
   the function returns FALSE, otherwise TRUE */
  while ( line[comptr] == ' ' )
    comptr++;

  if ( line[comptr] == '\0' )
    return FALSE;

  return TRUE;

}

/* ========================= nextch ============================= */

nextch(t)           /* parse next characters in command line */
char t[];
{
short int savptr,i;

#ifdef DEBUG
  printf ("\nnextch, input string is %s\n",t);
#endif

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
            value.  Characters are in line[comptr].  Value is idtval  */
short int idtcha, i;

  idtval = 0;
  for ( i=0; i<4; i++ )
    {
    idtval *= 16;
    idtcha = line[comptr];
    if ( idtcha >= '0' && idtcha <= '9')
      {
      idtval += (idtcha - 48) ;
      comptr++;
      }
    else
      {
      if ( idtcha >= 'A' && idtcha <='F' )
        {
        idtval += (idtcha - 55);
        comptr++;
        }
      }
    }
  return TRUE;
}

/* ============================>> getidt <<<============================= */
getidt()
{
/*  getidt - get the next 6 character ident from line into idt */
short int i;

  for ( i=0; i<6; i++ )
    {
    if ( (line[comptr] != ',') && (line[comptr] != 0) 
                               && (line[comptr] != '\n') ) 
      idt[i] = line[comptr++];
    else
      idt[i] = ' ';
    }

    idt[6] = 0;

#ifdef DEBUG
    printf ("\nGETIDT - Return string is %s\n",idt);
#endif

  return TRUE;
}

/* ==========================> fndidt <============================== */
fndidt ()
{
/* fndidt - searches the ref/def list for the ident in idt.  If matched,
            return index.  If not matched, return -1 */

short int idtp, refp, badsym, fndsym;

  fndsym = -1;

  for ( idtp=0; (idtp<=refptr) && (fndsym == -1) ; idtp++ )
    {
    badsym = FALSE;
    for ( refp=0; refp<6 && !badsym; refp++ )
      {
      if ( idt[refp] != refs[6*idtp+refp] )
        badsym = TRUE;
      }

    if ( !badsym ) 
      fndsym = idtp;
    }

#ifdef DEBUG
  printf ("FNDIDT - IDT is %s, FNDSYM is %x\n",idt,fndsym);
#endif

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

#ifdef DEBUG
  printf ( "Putting Symbol %s into ref/def, loc %x\n",idt,idtval);
#endif

  refptr++;
  refval[refptr] = idtval;

  for ( i=0; i<6; i++ )
    refs[(refptr*6)+i] = idt[i];

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

#ifdef DEBUG
   printf("SAVFIL - total size is %x, # modules is %d\n",totsiz,nomodu);
#endif

#ifdef DEBUG
   printf("special debug, memory[0f00] is %x, %x, %x, %x\n",
    memory[0xf00], memory[0xf01], memory[0xf02], memory[0xf03] );
#endif	

   if ( nomodu > 6 ) 			/* error check, limit # modules */
     {
     printf ("Error-Too many modules calculated, %d\n",nomodu);
     exit(999);
     }

   for ( i=0; i<nomodu; i++ )		/* do for each module */
     {

/*  Open the output file, binary output, file name incfil */
    printf("Creating OUTPUT file %s\n",incfil);

     if (!(chout = fopen(incfil,OPENWT)) )
       {
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

     if ( totsiz > mymax )		/* if this not last module */
     {
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
       }
     else
       {				/* last file!  */
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
     for ( j=0; j<128; j++ )
       {
       ouerr = fputc ( xrec[j], chout );
       if ( ouerr == EOF )
         {
         printf("Error writing to output file, %d\n",ouerr);
         exit(999);
         }
       }
/* Now write out this module.  Format the first 6 bytes.  These are:

         FFFF/0 - If more files follow/If last file
         mmmm - The number of bytes, including header
         nnnn - The start address to load                */
          
     if ( i == (nomodu-1) )              	/* Last Module */
       {
       xrec[0] = 0x00;
       xrec[1] = option;
       nnmodu  = totsiz + 6;
       xrec[2] = nnmodu >> 8;
       xrec[3] = nnmodu & 0xff;
       xrec[4] = start >> 8;
       xrec[5] = start & 0xff;
#ifdef DEBUG
       printf ("Packed xrec, start = %x, xrec[4] = %d\n",start,xrec[4]);
#endif
       }
     else				      /* Not the Last Module */
       {
       xrec[0] = 0xff;
       xrec[1] = option;
       xrec[2] = mymax >> 8;
       xrec[3] = mymax & 0xff;
       xrec[4] = start >> 8;
       xrec[5] = start & 0xff;
       }

/* write out the first 6 bytes, which is the E/A loader header */
     for ( j=0; j<6; j++ )
       {
       ouerr = fputc ( xrec[j], chout );
       if ( ouerr == EOF )
         {
         printf("Error writing to output file, %d\n",ouerr);
         exit(999);
         }
       }

/* now write out the actual data, whew */

     for ( j=0; j<thisfil; j++ )
       {
       memval = memory[start];
       start++;
       ouerr = fputc ( memval, chout );
       if ( ouerr == EOF )
         {
         printf ("Error writing to output file, %d\n",ouerr);
         exit(999);
         }
       }

/* pad last sector with zeroes (round up to even 256 byte sectors */
     for ( j=0; j<lastrec; j++ )
       {
       ouerr = fputc ( 0, chout );
       if ( ouerr == EOF )
         {
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
