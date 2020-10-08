lc1 >listfile -i:include/ -i:include/lattice/ -oram:loader.q loader.c
lc2 -v -oloader.o ram:loader.q
blink from :lib/lstartup.obj+loader.o to loader lib :lib/lc.lib,:lib/amiga.lib
copy loader sys:c/loader
