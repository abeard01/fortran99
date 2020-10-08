lc1 >listfile -i:include/ -i:include/lattice/ -oram:a99main.q a99main.c
lc2 -v -oa99main.o ram:a99main.q
lc1 >listfil1 -i:include/ -i:include/lattice/ -oram:a99pass1.q a99pass1.c
lc2 -v -oa99pass1.o ram:a99pass1.q
lc1 >listfil2 -i:include/ -i:include/lattice/ -oram:a99pass2.q a99pass2.c
lc2 -v -oa99pass2.o ram:a99pass2.q
lc1 >listfil3 -i:include/ -i:include/lattice/ -oram:a99psop.q a99psop.c
lc2 -v -oa99psop.o ram:a99psop.q
blink from :lib/lstartup.obj+a99main.o+a99pass1.o+a99pass2.o+a99psop.o to a99 lib :lib/lc.lib,:lib/amiga.lib
