/*                                                                   
                  a99:  The 99/4a and 9640 Cross Assembler
                                                                   
   Original version posted on BIX (non-TI 99 version).  Modified by A.Beard
   to have the following TI 99/4A features:                         
                                                                   
     1. Include line numbering, page numbering                     
     2. Remove restriction on AORG address                         
     3. Add EVEN psuedo opcode                                     
     4. Allow astericks to indicate comment lines                  
     5. Add TITL psuedo opcode                                     
     6. Add PAGE psuedo opcode                                     
     7. Add BYTE psuedo opcode                                     
     8. Add IDT  psuedo opcode                                     
     9. Change object code output to standard TI 99/4A format      
    10. Redid the way that TEXT strings are handled, so that text  
        and BYTE statements can be intermixed without adding extra 
        pads to null out to even strings.                          
    11. Add DORG psuedo opcode                                     
    12. Add correct checksum process (7 tag on records)            
    13. Add return (RT) instruction as B *R11                      
    14. Add DEF psuedo opcode                                      
    15. Increased number of symbols to 2000, total symbol table    
        size to 10000 characters.                                  
    16. Added symtyp table, to indicate type of symbol (e.g. 1 for 
        absolute), in preparation for RORG and REF's.              
                                                                   
  Program History:                                                 
                                                                   
  Version 2.0 - ti 99/4a initial version released to BIX on        
                1-may-87, by A.L.Beard                             

  Version 2.1 - added more features, including:                    
      1. Added REF statement                                       
      2. Added options for seperate listing (-l) and object (-o)   
         files.  NOTE:  The program now requires that you specify  
         a -l listing file, or a -o object file in order to get    
         one, they are not provided as default as before.          
      3. Added UNL (unlist) statement                              
      4. Extended LIST statement to include standard TI-99/4A form 
         of list statement, also extended LIST statement with file 
         name.                                                     
      5. Removed code at the end of BSS which forces an EVEN       
         statement.  Really messed up a lot of already existing    
         code.                                                     
      6. Forced last object code statement to be padded to 80      
         chars with blanks.                                        
      7. Rewrote the TEXT code to handle the multiple delimeter    
         case of '' (to mean a single ')                           
      8. Hashed the symbol table to gain execution speed (it really
         got slow on 20kb files).   Improved things on the order   
         of three times.  Timings showed that a 3800 line file on  
         the old "non-hashed" assembler took 6 min (on Amiga, with 
         Lattice -C-).  The "hashed" version took 2 min.           
      9. Allowed MACRO lines to have comments starting with blanks 
     10. Added new MACRO listing command "MOBJECT", which allows   
         the original line + object to be listed.  made it the     
         default.                                                  
     11. Increased macro space from 20 to 200 (You may want to     
         decrease this in your header file, this causes a 40k      
         increase in executable module size.                       
     12. Changed the way local branch names are assigned within    
         macros.  Old method bumped name on every macro call       
         regardless of whether any local labels were used, causing 
         an unecessary overflow error about every 1400 macro in-   
         vocations.                                                

   Version 2.2 - Minor updates to correct problems found by Ron    
                   Lepine (thanks Ron).   Removed error for extra    
                   operands present, no longer makes sense for the   
                   new comment scheme.                               

   Version 2.3 - More additions:                                   
       1. Add COPY (INCLUDE) file feature.                         

   Version 2.4 - More additions:                                     
       1. IF/THEN/ELSE/ENDIF                                         
       2. Correct UNL psuedo op. 

   Version 2.5 - Made some changes for MSDOS compatibility.  This version
                 never released.

   Version 2.6 - More additions:
       1. Extend IF to IFEQ,IFNE,IFGT,IFGE,IFLT,IFLE, IFD, IFND     
            (retained IF as IFEQ for compatibility)
       2. Additions for extended 9900/99000 instruction set,
            Thanks for information from Tony Lewis concerning these
            items.
       3. Binary search added for opcode search.  Improved performance
            of assembler by about 25%.

   Version 2.6a -  (courtesy Ron Lepine)
        1. Moved psop() to it's own file to allow DLC to optimize
             pass2.  It had errors on psop().
        2. In a99pass1.c I changed ctime() and t to std UNIX
             definition so all my compilers could use it. The
             way it was not all would compile without warnings.
        3. Removed *modf in a99.h.  I dont see where it's used.
        4. Removed external function declarations                       

   Version 2.7 - (Al Beard again)

        1. Fixed LWPI instruction so that relative workspace pointers
           could be used.
        2. Changed the last object record to include a sequence number.
           This allowed the object record to be a full 80 characters
           (not blank filled at end) which was giving grief with ti-99
           kermit.
        3. Fixed ELSE processing.  Previously worked erratically.
        4. Added psuedo RT instruction.

   Version 2.8 - (Al Beard again)

        1. Detab listing file so that output file prints correctly.

   Version 2.8a - Ron Lepine

        "Pretty" code via C beautifier.  Changed many int's to short
        or Long ints for compiler compatibilities.

   Version 2.8b - (Al Beard again)

        1. Fixed a bug with EVEN, was incrementing twice (no, I don't
           know where the bug came from, very strange).
        2. Force an "EVEN" directive whenever an instruction or DATA
           statement is executed.  Is more consistent with the TI-99
           E/A assembler.
        3. Got rid of outmac subroutine, replaced with outcom subroutine.
           They basically did the same thing, and outcom detabs the output
           line properly so the listing won't be crunched if tabs are input.

   Version 2.9 - (Al Beard again)
  
        1. Add new 'compress' option, to produce normal TI-99 or GENEVE
           compressed object.
        2. Due to compress option, changed the object code format to
           be 80 bytes per record, period.  The old format was 80 bytes
           of object followed by a n/l, followed by 80 bytes of object,
           followed by a n/l, etc.   The new format is just 80 bytes
           of object.
        3. Fixed the ':' colon last record to fill the record.  There
           was a bug which was only partially filling a record.
        4. Removed debug statements.

   Version 2.10 (Al Beard again):

        1. Generate long references, if mode option chosen
        2. If object not chosen, don't output to object file
        3. emtdef changed to always capitalize def'd names

   Version 2.11 (Al Beard again):
        1. Fix emtdef for non-extended assembler
        2. Allow option letters to be in caps as well as lower case
           (MSDOS style)

   Version 2.12 (Al Beard)
        1. Remove warning on unused refs
        2. Implement new "X" opcode, for use with segmented loader.
*/

#define MAIN TRUE

#include <stdio.h>
#include "a99.h"
/* =============================>>> MAIN  <<<============================= */
main(argc,argv)

int argc;
char *argv[];
{

int argnum, fndfile, errstm, iptr, optr;
char buff[80];

    printf("\n%s\nCopyright 1988/1990 by Al Beard/Ron Lepine\n",VERSION);

    /*  Setup for parsing the input file, error file,       */
    /*  object file, and listing file                       */

    errstm  = FALSE;    /* error while parsing args         */
    fndfile = FALSE;    /* found main file arg or not       */
    liston  = FALSE;    /* found listing file arg or not    */
    objon   = FALSE;    /* found object file name or not    */
    registers = TRUE;   /* register definitions supported   */
    ex9900  = TRUE;     /* 9900 extensions supported        */
    ex99000 = FALSE;    /* 99000 extensions supported       */
    compobj = FALSE;    /* do not compress object code      */
    extended  = FALSE;  /* Extended Assembler               */
    xopcode = FALSE;    /* generate segmented X opcodes     */

    /*  Parse each argument in the command string */

    for (argnum = 1 ; argnum < argc ; ++argnum){        
        strcpy (buff, argv[argnum]);

        if (buff[0] == '-'){ 		/* its a listing file */            
            if ((buff[1] == 'l' || buff[1] == 'L' ) && !liston){                
                liston = TRUE;
                iptr = 2;
                optr = 0;
                while (buff[iptr] != 0)
                    listf[optr++] = buff[iptr++];
                listf[optr] = 0;
                }/* its an object file */

            else if ((buff[1] == 'o' || buff[1] == 'O' ) && !objon){
                objon = TRUE;
                iptr = 2;
                optr = 0;
                while (buff[iptr] != 0)
                    objf[optr++] = buff[iptr++];
                objf[optr] = 0;
                }
            /* parameters? */
            else if (buff[1] == 'p' || buff[1] == 'P' ){              
                iptr = 2;
                while (buff[iptr] !=0){ /* register definitions */
                    if (buff[iptr] == 'n' || buff[iptr] == 'N' )
                        registers = FALSE;
                    /* 9640 opcode extensions */
                    else if (buff[iptr] == 'g' || buff[iptr] == 'G' )
                        ex9900 = TRUE;
                    /* 99000 opcode extensions */
                    else if (buff[iptr] == 'h' || buff[iptr] == 'H' ){
                        ex9900 = TRUE;
                        ex99000 = TRUE;
                        }
                    else if (buff[iptr] == 'c' || buff[iptr] == 'C' ){
                        compobj = TRUE;
                        }
		    else if (buff[iptr] == 'm' || buff[iptr] == 'M' ){
                        extended  = TRUE;
                        }
                    else if (buff[iptr] == 'x' || buff[iptr] == 'X' ){
                        xopcode   = TRUE;
                        }
                    else{                        
                        printf("Unrecognized Parameter %c\n",buff[iptr]);
                        errstm = TRUE;
                        }
                    iptr++;
                    }
                }
            else
                errstm = TRUE;      /* unrecognized parameter */
            }
        else if (!fndfile)          /* must be input file */{            
            fndfile = TRUE;
            strcpy (inputf, buff);
            }
        else                        /* don't know what it is, error */
            errstm = TRUE;
        }

    /*  If any error in statement, or input file not found, display help */
    if (errstm || !fndfile){        
        printf("\nUsage: a99 [-ohex object] [-llisting file] [-poptions] filename\n");
        printf(" File extensions are assumed, do not specify them:\n");
        printf("         .a99   for source file\n");
        printf("         .l99   for listing file\n");
        printf("         .x99   for hexadecimal object file\n");
        printf(" The listing file, object file, and options are all optional,\n");
        printf(" if they are not specified then they will not be produced.\n");
        printf("\n");
        printf(" The options are specified as single letter small characters:\n");
        printf("\n");
        printf("       n  - do not produce register definitions\n");
        printf("       g  - produce TMS9900 extended instructions\n");
        printf("       h  - produce TMS99000 extended instructions\n");
        printf("       c  - compress object code\n");
        printf("       m  - generate long name references\n");
        printf("       x  - generate special X mode ref chains\n");
        printf("\n");
        printf(" For example:\n");
        printf("\n");
        printf("       a99 -otest -ltest -pnc test\n");
        printf("\n");
        printf(" will assemble the file test.a99, place the object in the\n");
        printf(" test.x99, create a listing file called test.l99, and ass-\n");
        printf(" emble using the n (no register) and c (compress) options\n");
        printf("\n");
        exit(0);
        }

    /*  Otherwise, start execution of assembler */

    init();         		/* initialize */
    passnr = 1;     		/* start with pass 1 */
    pass1();        		/* pass 1 */
    endp1();        		/* cleanup pass 1 processing */
    fclose(chin);   		/* close the input file */
    if (!(chin = fopen(inputf,OPENRD))){        
        printf("Fatal Error: Can't open input file %s for pass2, error %d\n",
        inputf, chin);
        exit(999);
        }
    passnr = 2; 		/* continue with pass 2 */
    pass2();    		/* start it */
    aquit();    		/* cleanup and exit */
}
