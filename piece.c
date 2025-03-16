#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"
#include "/sys/src/games/eui.h"

/* FIXME: z and s pieces definitely should not wobble when rotating */
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

enum{
	Cline = 0xf5,
};

/* FIXME: draw level, line clears, score */
/* FIXME: freeze screen on game over + print message instead of immediate exit */

static void
drawbackground(void)
{
	int n, m, y;
	u32int c, *p, *pe;

	memset(pic, 0, Vheight * Vwidth * sizeof *p);
	c = palette[Cline];
	p = (u32int *)pic;
	for(y=Nrow-Wheight; y<Nrow; y++){
		p += Wside * Block;
		for(pe=p+Ncol*Block; p<pe; p++)
			*p = c;
		p += Wside * Block;
		m = y == Nrow - 1 ? Block - 2 : Block - 1;
		for(n=0; n<m; n++){
			p += Wside * Block;
			for(pe=p+Ncol*Block; p<pe; p+=Block)
				*p = c;
			p[-1] = c;
			p += Wside * Block;
		}
	}
	p += Wside * Block;
	for(pe=p+Ncol*Block; p<pe; p++)
		*p = c;
}

static void
drawplayfield(void)
{
	int n, y;
	u32int *p, *pe;
	char fc, *f, *fs, *fe;
	uchar *sp;

	p = (u32int *)pic;
	fs = playfield + Wwidth * (Nrow - Wheight);
	for(y=Nrow-Wheight; y<Nrow; y++){
		fe = fs + Ncol;
		for(n=0; n<Block; n++){
			p += Wside * Block;
			for(f=fs; f<fe; f++){
				if((fc = *f) == 0){
					p += Block;
					continue;
				}
				sp = sprites[fc - 1] + n * Block;
				for(pe=p+Block; p<pe; p++)
					*p = palette[*sp++];
			}
			p += Wside * Block;
		}
		fs += Ncol;
	}
}

static void
drawfour(int x, int y, int rot, int type)
{
	int f, n, m, k;
	uchar *sp;
	u32int *l, *s, *p, *pe;

	p = (u32int *)pic + (y - Nstartrow + Nextrarows) * Vwidth * Block;
	p += (Wside + x) * Block;
	l = p;
	f = fours[type][rot];
	for(k=0, m=1<<(Nside*Nside-1); m>0; m>>=1){
		s = p;
		if(f & m){
			sp = sprites[type];
			for(n=0; n<Block; n++){
				if(p >= (u32int *)pic)
					for(pe=p+Block; p<pe; p++)
						*p = palette[*sp++];
				else
					p += Block;
				p += Vwidth - Block;
			}
		}
		p = s + Block;
		if(++k == Nside){
			k = 0;
			l += Vwidth * Block;
			p = l;
		}
	}
}

static void
drawsides(void)
{
	int y, *p;

	if(held != -1)
		drawfour(-4, Nrow / 2, 1, held);
	for(y=1, p=next; p<next+nelem(next); p++, y+=5)
		drawfour(Wwidth, Nstartrow - Nextrarows + y, 1, *p);
}

static void
drawpiece(void)
{
	if(cur == nil)
		return;
	drawfour(cur->x, cur->y, cur->rot, cur->type);
}

static void
drawui(void)
{
}

void
redraw(void)
{
	drawbackground();
	drawplayfield();
	drawpiece();
	drawsides();
	drawui();
	flushmouse(1);
	flushscreen();
}
