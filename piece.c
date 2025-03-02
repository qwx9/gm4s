#include <u.h>
#include <libc.h>
#include <thread.h>
#include <pool.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"
#include "/sys/src/games/eui.h"

int fours[NF][Nrot] = {
	[FI] {
		0b0000111100000000,
		0b0010001000100010,
		0b0000000011110000,
		0b0100010001000100,
	},
	[FJ] {
		0b1000111000000000,
		0b0110010001000000,
		0b0000111000100000,
		0b0100010011000000,
	},
	[FL] {
		0b0010111000000000,
		0b0100010001100000,
		0b0000111010000000,
		0b1100010001000000,
	},
	[FO] {
		0b0000011001100000,
		0b0000011001100000,
		0b0000011001100000,
		0b0000011001100000,
	},
	[FS] {
		0b1100011000000000,
		0b0010011001000000,
		0b0000110001100000,
		0b0100110010000000,
	},
	[FT] {
		0b0100111000000000,
		0b0100011001000000,
		0b0000111001000000,
		0b0100110001000000,
	},
	[FZ] {
		0b0110110000000000,
		0b0100011000100000,
		0b0000011011000000,
		0b1000110001000000,
	},
};

static u32int cols[NF] = {
	[FI] DCyan,
	[FJ] DBlue,
	[FL] DOrange,
	[FO] DYellow,
	[FS] DGreen,
	[FT] DPurple,
	[FZ] DRed,
};
static Image *piece[NF];

enum{
	Nline = Ncol * Block,
};

static void
drawplayfield(void)
{
	int w;
	u32int c, *s, *p, *pe;
	char fc, *f, *fe;

	w = Block * 1;
	p = (u32int *)pic;
	f = playfield + Ncol * (Nrow - Nstartrow - Nextrarows);
	for(s=p, fe=playfield+nelem(playfield); f<fe; f++){
		if((fc = *f) == 0)
			//c = cols[nrand(nelem(cols))];
			c = 0xff000000;
		else
			c = cols[fc - 1];
		for(pe=p+w; p<pe; p++)
			*p = c;
		if(p - s == Ncol * w){
			p += Ncol * w * (Block - 1);
			s = p;
		}
	}
}

static void
drawpiece(void)
{
	int f, x, n, w;
	u32int c, *l, *s, *p, *pe;

	if(cur == nil)
		return;
	s = (u32int *)pic;
	s += (cur->y - Nstartrow + Nextrarows) * Nline * Block + cur->x * Block;
	l = s;
	c = cols[cur->type];
	f = fours[cur->type][cur->rot];
	w = Block * 1;
	for(n=0, x=1<<(Nside*Nside-1); x>0; x>>=1){
		if(s >= (u32int *)pic && f & x)
			for(p=s, pe=p+w; p<pe; p++)
				*p = c;
		if(++n == Nside){
			l += Block * Ncol * w;
			s = l;
			n = 0;
		}else
			s += w;
	}
}

static void
vscalepic(void)
{
	int n;
	u32int *p, *s, *e;

	n = 1 * Ncol * Block;
	for(s=(u32int*)pic, e=s+n*Wheight; s<e; s=p)
		for(p=s+n; p<s+n*Block; p+=n)
			memcpy(p, s, n * sizeof *p);
}

void
redraw(void)
{
	drawplayfield();
	drawpiece();
	vscalepic();
	flushmouse(1);
	flushscreen();
}

void
initimg(void)
{
	u32int col, *c;
	Rectangle r;
	Image **i, **ie;

	r = Rect(0, 0, Block, Block);
	for(c=cols, i=piece, ie=i+nelem(piece); i<ie; i++, c++){
		col = *c;
		if((*i = allocimage(display, r, XRGB32, 0, col)) == nil)
			sysfatal("allocimage: %r");
		*c = col & 0xff << 24 | col >> 8 & 0xffffff;
	}
}
