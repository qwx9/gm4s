</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=gm4s
HFILES= dat.h fns.h /sys/src/games/eui.h
OFILES=\
	dat.$O\
	game.$O\
	gm4s.$O\
	nanosec.$O\
	piece.$O\
	/sys/src/games/eui.$O\

</sys/src/cmd/mkone

%.$O: %.c
	$CC $CFLAGS -o $target $stem.c
