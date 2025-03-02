</$objtype/mkfile
BIN=$home/bin/$objtype
TARG=gm4s
OFILES=\
	game.$O\
	gm4s.$O\
	piece.$O\
	eui.$O\

HFILES= dat.h fns.h
</sys/src/cmd/mkone
eui.$O: /sys/src/games/eui.c
	$CC $CFLAGS $prereq
