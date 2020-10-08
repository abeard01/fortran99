                                                            15-May-1989

  To whom it may concern:

  I took another look at a99 in conjunction with some improvements
  to the 99/9640 FORTRAN linker.  I decided that compressed object
  could be added easily, so I added it to the a99 assembler, the
  cross-loader (and subsequently the 99/9640 FORTRAN linker).

  It also contains a few more bug fixes, removal of some debug code,
  and a few others.   I'll pass this along to Ron, he is my "MSDOS"
  expert and may get an MSDOS compatible version going.

  Only thing I ask, if you do additions or corrections to this program, then
  please post the changes so that everyone can enjoy.   I'll continue to
  keep the master "real" version, please feed back to me and I'll be happy
  to post for the world.

  This archive contains the following files:

           a99.h        header file for a99
           a99proto.h   header file containing def's for c modules
           a99main.c    main program
           a99pass1.c   pass 1 of the compiler
           a99pass2.c   pass 2 of the compiler
           a99psop.c    psuedo-opcode processing (e.g. AORG, RORG, etc.)
           loader.c     the loader program

  Enjoy!
                 al beard
