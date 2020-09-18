Tony,

Sorry for the slow response on your note.  I am releasing 9640 FORTRAN (finally)
at the TICOFF, and preparations have been intense.

>Memory Mapped vs. DSR
Looking over your note, of course I can ignore the DSR (dummy me, hadn't
thought of that).  The TI-BASIC routines are possible in the DSR, but
somewhat cumbersome.  I was thinking more along the lines of replacing
the console ROM's with new ROM's (just replacing the code for the RADIX
100 implementation with IEEE implementation).  This doesn't handle the GPL
stuff, though (although I wonder if those could be rewritten as well).

I'll have to give it more thought.  I'm a little worried about writing a
DSR, haven't done it yet and don't know of any specific documents that talk
about it, although Mike Dodd has done a lot with them.

>Initialization
Ok, no initialization code (great!).

>Serious applications in 99 FORTRAN or C99
I agree.  99 FORTRAN can be modified easily to accomodate the chip, mostly
libraries, the compiler would need new string to f.p. conversion routine,
and the runtime support would need new string->f.p., f.p.-> string.

Nothing difficult.

The only suggestion on "replacing" the ROM in the TI CONSOLE is that I had
heard (don't remember where) that it could be done easily without any
surgery to the console by using the right kind of EPROM chip, the data line
drivers on the TI ROM chips used are so weak they are easily overridden
by an external EPROM.

But, lets keep it simple for now.  I like your design and the use of the
68881 chip.

Any luck on getting a chip?

  Looking forward to the project,

     al beard
