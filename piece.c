#include <u.h>
#include <libc.h>
#include <thread.h>
#include <pool.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"
#include "/sys/src/games/eui.h"

/* FIXME: stabler rotations? */
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

/* FIXME: draw level, line clears, score */
/* FIXME: freeze screen on game over + print message instead of immediate exit */

static void
drawplayfield(void)
{
	u32int c, *s, *p, *pe;
	char fc, *f, *fe;

	memset(pic, 0, Vwidth * Vheight * sizeof *p);	/* FIXME: sides */
	p = (u32int *)pic + Wside * Block;
	f = playfield + Wwidth * (Nrow - Wheight);
	for(s=p, fe=playfield+nelem(playfield); f<fe; f++){
		if((fc = *f) == 0)
			//c = cols[nrand(nelem(cols))];
			c = 0xff000000;
		else
			c = cols[fc - 1];
		for(pe=p+Block; p<pe; p++)
			*p = c;
		if(p - s == Wwidth * Block){
			p += Vwidth * (Block - 1) + 2 * Wside * Block;
			s = p;
		}
	}
}

static void
drawfour(int x, int y, int rot, int type)
{
	int f, m, n;
	u32int c, *l, *s, *p, *pe;

	s = (u32int *)pic;
	s += (y - Nstartrow + Nextrarows) * Vwidth * Block + x * Block;
	l = s;
	c = cols[type];
	f = fours[type][rot];
	for(n=0, m=1<<(Nside*Nside-1); m>0; m>>=1){
		if(s >= (u32int *)pic && f & m)
			for(p=s, pe=p+Block; p<pe; p++)
				*p = c;
		if(++n == Nside){
			l += Vwidth * Block;
			s = l;
			n = 0;
		}else
			s += Block;
	}
}

static void
drawui(void)
{
	int y, *h;

	if(held != -1)
		drawfour(-1, Nrow / 2, 1, held);
	for(y=1, h=hist+1; h<hist+nelem(hist); h++, y+=5)
		drawfour(Wside + Wwidth, Nstartrow - Nextrarows + y, 1, *h);
}

static void
drawpiece(void)
{
	if(cur == nil)
		return;
	drawfour(Wside + cur->x, cur->y, cur->rot, cur->type);
}

static void
vscalepic(void)
{
	int n;
	u32int *p, *s, *e;

	n = Vwidth;
	for(s=(u32int*)pic, e=s+n*Vheight; s<e; s=p)
		for(p=s+n; p<s+n*Block; p+=n)
			memcpy(p, s, n * sizeof *p);
}

void
redraw(void)
{
	drawplayfield();
	drawpiece();
	drawui();
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
