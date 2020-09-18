#include        <stdio.h>
#include        <environ.h>
#include        "c.h"
#include        "expr.h"
#include        "gen.h"
#include        "cglbdec.h"
#include        "version.h"
#include        "proto.h"

/*
 *      TMS9900 C Compiler
 *
 *      This is the main entry point for the TMS9900 C compiler.
 *
 *      Calling Sequence:
 *
 *           tic filename
 *
 *      The filename will be stripped of its extension (usually .c), and
 *      the following output files will be created:
 *
 *           filename.S      -   ti-99/4a assembly source
 *           filename.L      -   listing file
 *
 *  Current Compilation Switches:
 *
 *      a   -   tasm compatibility      (default: TRUE)
 *      o   -   Peephole Optimization   (default: TRUE )
 *      r   -   Register Optimization   (default: TRUE )
 *      l   -   Listing                 (default: FALSE )
 *      i   -   only use with + switch, defines another directory to
 *              search for include.
 *      q   -   quiet flag - eliminates output
 *      s   -   optional place to put source file (e.g. +sDSK5.)
 *
 *  Use a + sign to turn option ON,  a - sign to turn option OFF, e.g.:
 *
 *      tic +ol -ic:\includes cmain.c
 *
 *  Means:
 *
 *    compile cmain.c with optimization, listing, no c99 compatibility.
 *
 *  Copyright 1991, 1993 by LGMA Products
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
/* Change History:
**
** Add new revisions to top.
**
**     Who           Date            Reason
**     ===           =========       =======================================
**     ALB            1-MAR-93       Implement S option, remove references
**                                   to "9" option.
**     ALB           23-SEP-92       Sense of "Q" switch was reversed.
 */

void summary()
{	printf("\n -- %d errors found.\n",total_errors);
	fprintf(output,"\tEVEN\n");

	if ( listflag )
          fprintf(list,"\f\n *** global scope symbol table ***\n\n");

        list_table(&gsyms,0);

	if ( listflag )
          fprintf(list,"\n *** structures and unions ***\n\n");

        list_table(&tagtable,0);
	fprintf(output,"\tEND\n");
}

void makename(s,e)
char    *s, *e;
{       
#ifdef GENEVE
	while (*s != 0 && *s != '_' )
#else
	while(*s != 0 && *s != '.' && *s != '_' )
#endif
                ++s;
	if ( *s != 0 )
	        ++s;
        while(*s++ = *e++);
}
void closefiles()
{       if ( input != 0 )
          fclose(input);
        if ( output != 0 )
          fclose(output);
        if ( list != 0 )
          fclose(list);
        if ( copy != 0 )
            fclose(copy);
}

/* remove options */
void roptions(s)
char	*s;
{
	while ( *++s != 0 )
          {
          switch ( *s )
            {
            case 'o':
	    case 'O':
              optflag = FALSE;		/* o - no peephole optimization */
              break;
            case 'l':
	    case 'L':
              listflag = FALSE;		/* l - no listing */
              break;
            case 'r':
            case 'R':
              regflag  = FALSE;         /* r - no register optimization */
              break;
            case 'w':
            case 'W':
              windowflag = FALSE;	/* format output for windows */
              break;
            case 'a':
            case 'A':
              copyflag = TRUE;         /* a - copy file for ref/def */
              break;
            case 'q':                   /* q - quiet flag */
            case 'Q':
              quiet    = FALSE;
              break;
            default:
              printf("Error - Bad Option %s\n", s);
              break;
	    }
	  }
}
/* handle add options */

void  aoptions(s)
char	*s;
{
	while ( *++s != 0 ) {
          switch ( *s ) {
/*  turn optimization ON */
            case 'o':
            case 'O':
              optflag = TRUE;		/* o - peephole optimization */
              break;

/*  turn listing ON  */
            case 'l':
            case 'L':
              listflag = TRUE;		/* l - listing */
              break;

            case 'r':
            case 'R':
              regflag  = TRUE;          /* r - register optimization */
              break;

            case 'w':
            case 'W':
              windowflag = TRUE;	/* w - format output for windows */
              break;

/*  multiple include file directories, stack in inc_files  */
            case 'i':
            case 'I':
              ++s;
              while ( *s != 0 && *s != ' ' )
                *incptr++ = *s++;
              *incptr++ = 0;
              s--;
              break;

            case 'a':
            case 'A':
              copyflag = FALSE;
              break;

            case 'q':
            case 'Q':
              quiet    = TRUE;
              break;

            case 's':
            case 'S':
              ++s;
              while ( *s !=0 && *s != ' ' ) {
                *srcptr++ = *s++;
	      }
              *srcptr++ = 0;
              s--;
              break;

/*  bad option, just skip it */
            default:
              printf("Error - Bad Option %s\n", s);
              break;
	    }
	  }
}

/* open necessary input, output, and optionally listing file */
int     openfiles(s)
char    *s;
{       strcpy(infile,  s);		/* move base file name into */
        strcpy(listfile,s);		/* individual file names    */
        strcpy(outfile,src_files);      /* redirect for source file */
        strcat(outfile, s);
        strcpy(copyfile,s);

        makename(listfile,"L");		/* listing file name */
        makename(outfile, "S");		/* output source file name */
        makename(copyfile,"I");		/* output copy file name */

        if( (input = fopen(infile,INPUTMODE)) == 0) {
                printf(" **can't open %s\n",infile);
                return FALSE;
        }
        
        if( (output = fopen(outfile,OUTPUTMODE)) == 0 ){
                printf(" **can't open %s\n", outfile);
                closefiles();
                return FALSE;
                }
                
        fprintf(output,"* TIC %s (c) 1991, 1993 by LGMA Products\n",VERSION);

        if ( copyflag ) {
          if( (copy = fopen(copyfile,OUTPUTMODE)) == 0 ) {
             printf(" ** can't open %s\n", copyfile );
             closefiles();
             return FALSE;
          }else{
             fprintf(output,"\tCOPY \"%s\"\n",copyfile);
             }
        } 

        if ( listflag ) {
          if( (list = fopen(listfile,LISTINGMODE)) == 0) {
                printf(" ** can't open %s\n",listfile);
                closefiles();
                return 0;
                }
        }
        return TRUE;
}

void init()
/*  initializes certain variables */
{
	optflag  =   TRUE;           /* peephole optimization ON */
	listflag =   FALSE;          /* listing OFF */
	regflag  =   TRUE;           /* register optimization ON */
        copyflag =   FALSE;          /* copy flag OFF */
	incptr   =   inc_files;      /* assume no includes */
        *incptr  =   0;
        srcptr   =   src_files;      /* assume no source files */
        *srcptr  =   0;
        manlis   =   LOCALIST;	     /* which list is in memory */
        windowflag = FALSE;	     /* format output for windows */
        quiet    =   TRUE;           /* quiet flag on compilation */
	assembly =   FALSE;	     /* set not processing assembly */
}
void helpmenu()
/* helpmenu - displays a help screen to the user if he doesn't type in
   any parameters                                                      */
{  
printf("TIC [options] filename\n\n");
#ifdef GENEVE
printf("The filename will be stripped of it's _C extension and\n");
printf("the following files will be created:\n");
printf("    filename_S  -  Assembly Source\n");
printf("    filename_L  -  Listing\n\n");
#else
printf("The filename will be stripped of it's .c extension and\n");
printf("the following files will be created:\n");
printf("    filename.s  -  Assembly Source\n");
printf("    filename.l  -  Listing\n\n");
#endif
printf("Options Are:\n");
printf("    A   -   TASM Compat    default: ON\n");
printf("    R   -   Register Opt   default: ON\n");
printf("    O   -   Peephole Opt   default: ON\n");
printf("    L   -   Listing ON/OFF default: OFF\n");
printf("    Q   -   Quiet Flag     default: ON\n");
printf("    I   -   Include File Specification\n");
#ifdef GENEVE
printf("    S   -   Output File Redirect (e.g. DSK5.)\n");
#else
printf("    S   -   Output File Redirect (e.g. B:)\n");
#endif
printf("Use + or - to turn option on (+) or off (-). For example:\n\n");
#ifdef GENEVE
printf("    TIC +OL +IHDS1.INCLUDE. +IB: +SDSK5. CMAIN_C\n\n");
printf("will create CMAIN_S and CMAIN_L.  CMAIN_S requires assembly\n\n");
#else
printf("    TIC +OL +IC:\\INCLUDES\\ +SC:\\TEMP\\ CMAIN.C\n\n");
printf("will create CMAIN.S and CMAIN.L.  CMAIN.S requires assembly\n\n");
#endif
exit(999);
}
void main(argc,argv)
int     argc;
char    **argv;
{	
	printf("\nTIC C Compiler %s (c) 1991, 1993 by LGMA Products\n",VERSION);
        init();

        if ( argc == 1 )
          helpmenu();

        swap(LOCALIST);			/* swap in local symbol memory */

        while(--argc) {
                if      ( **++argv == '-' )
                        roptions(*argv);

                else if ( **argv == '+' )
                        aoptions(*argv);

                else if ( **argv == '?' )
                        helpmenu();

                else if( openfiles(*argv)) {
			*incptr = 0;		/* zap last file */
                        lineno = 0;
			printf("Processing %s\n",infile);
                        initsym();
                        getch();
                        getsym();
                        compile();
                        summary();
                        release_global();
                        closefiles();
                        }
                }
}
