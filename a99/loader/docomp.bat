lc1 >listfile -isys:include/ -i:include/lattice/ -oram:loader.q loader.c
lc2 -v -oloader.o ram:loader.q
blink from :lib/lstartup.obj+loader.o to loader lib :lib/lc.lib,:lib/amiga.lib
