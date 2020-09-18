
To whom it may concern

These are the source files for packages called:

99/9640 FORTRAN   -   FORTRAN for TI-99 and MYARC GENEVE 9640 computers
tic                   TI/C C compiler for TI-99 and MYARC GENEVE 9640 computers

There are a lot of directories here.

FORTRAN Compiler

  NCOMPILE    -  is the actual compiler
                  FORTRAN1.A99 -   Is the compiler and interpreter for FORTRAN2
                  FORTRAN2.A99 -   Is a modeled 64 register/64 bit machine that interpretes FORTRAN code
                  FORTRAN3.A99 -   Generates the code based on output of FORTRAN1/2
                  FORTRAN4.A99 -   Peephole optimizer for FORTRAN3 output
                  FORTRAN5.A99 -   common data area
                  FORTRAN6.A99 -   Geneve specific support routines
                  FORTRAN7.A99 -   Version identifier

  FORTRAN Libraries
                  SPLIB        -   Single precision (i.e.32 bit) floating point library
                  DPLIB        -   DOuble precision (i.e. 64 bit) floating point library
                  INLIB        -   Integer (i.e. 16 bit) library
                  CPLIB        -   Complex floating point library
                  EPLIB        -   Extended precision (complex data type) library
                  GRAPHICS     -   Graphic support libraries
                  LIBRARY      -   OS level library support routines and misc.
                  SPECRO       -   The special library functions embedded in FORTRAN
                  WINLIB       -   Set of windows library functions that do various things.  Used
                                   in 9640 Windows


C Compiler (TIC)
                  tic          -  compiler
                  Note that library for tic was created by Clint Pulley, based on his Tiny-C
                  compiler but recompiled with tic.   I don't have code for this.

Support Functions
                  DEBUG        -  Simplistic debugger, orignal supplied with 99 FORTRAN
                  EDITOR       -  Original FORTRAN editor supplied with 99 FORTRAN
                  EQUATES      -  9640 MDOS equates
                  EXCTIME      -  Execution time support.  Interprets "FORMAT" statements,
                                  other support functions.
                  FDEBUG       -  Symbolic debugger supplied with 9640 FORTRAN
                  GOBJECTS     -  Build scripts for linker/loader
                  LINKER       -  The TI-99 FORTRAN linker
                  OBJECTS      -  Loader files 
                  MENU         -  Boot loader for FORTRAN stand alone tasks

Other
                  IEEE         -  Prototype code for IEEE Floating point support, worked on
                                  this with Tony - he was migrating Motorola 68881 FP Chip to
                                  MYARC as a FP co-processor card.   Never completed.


Demos/Utilities

                 ARCHIVE       -  Archive program for hard drive
                 BACKUP        -  Backup program
                 CALC          -  Spreadsheet program
                 CALCULAT      -  Seems to be another spreadsheet program, copy of CALC
                 DEMOS         -  Contains several demo programs
                                  BOXES   -  make boxes
                                  DRIVERS -  make a sine wave
                                  FILEZAP -  File editor
                                  FRACTALS - Make fractals
                                  M39      - Mod 39 converter
                                  MOUSES   - Handle Bruce Hellstrom's mouse
                                  unfortunatly there is some corruption in these files
                 PLOT          -  Plotting program
                 SHOW          -  Display Amiga IFF files
                 TRANSFOR      -  Transform files from one type to another
                 UTILITY       -  various utilities embedded in one file

A.L.Beard
9/18/2020
                  