#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"
#include "/sys/src/games/eui.h"

char playfield[Ncol * Nrow];
Current *cur;

enum{
	Nlineperlvl = 10,
	Timeinc = BILLION / 10.0,
};
static vlong ncleared;
static int held = -1;

static int bfield[Nrow];

static u32int
trand(void)
{
	static u32int x = 0xffffffff;

	if(x == 0xffffffff)
		x = rand();
	x = x * 0x41c64e6d + 0x3039 >> 10 & 0x7fff;
	return x;
}

static int
getpiece(void)
{
	int i, *h, r;
	static int hist[4] = {FZ, FZ, FS, FS};

	for(r=i=0; i<5; i++){
		r = trand() % 7;
		for(h=hist; h<hist+nelem(hist); h++)
			if(*h == r)
				break;
		if(h == hist + nelem(hist))
			break;
		r = trand() % 7;
	}
	hist[3] = hist[2];
	hist[2] = hist[1];
	hist[1] = hist[0];
	hist[0] = r;
	return r;
}

static void
spawn(void)
{
	static Current cur0;

	memset(&cur0, 0, sizeof cur0);
	cur0.type = getpiece();
	cur0.rot = Up;
	cur0.x = Ncol / 2 - 2;
	cur0.y = Nstartrow - Nextrarows - 1;
	cur = &cur0;
	if(collide(cur->x, cur->y, cur->rot))
		gameover();
}

/* FIXME: gm4s: freeze: overlapping piece type 2 0,21 at 0,21:
 * and I piece horizontal clipped past the wall to the right prior */

int
collide(int x, int y, int rot)
{
	int n, m, s, b, l, *pp, f;

	switch(x){
	case -3:     b = 0b1110111011101110; s = Ncol - 1; m = 1; break;
	case -2:     b = 0b1100110011001100; s = Ncol - 2; m = 3; break;
	case -1:     b = 0b1000100010001000; s = Ncol - 3; m = 7; break;
	case Ncol-3: b = 0b0001000100010001; s = 1; m = 14; break;
	case Ncol-2: b = 0b0011001100110011; s = 2; m = 12; break;
	case Ncol-1: b = 0b0111011101110111; s = 3; m = 8; break;
	default: b = 0; s = Ncol - Nside - x; m = 15; break;
	}
	f = fours[cur->type][rot];
	pp = bfield + y;
	for(n=12; n>=0; n-=4, y++){
		if(y >= Nrow)
			l = 15;
		else if(x >= Ncol - 3)
			l = *pp++ << s & m;
		else
			l = *pp++ >> s & m;
		b |= l << n;
	}
	return b & f;
}

static void
updatelevel(void)
{
	double t;

	if(++ncleared % Nlineperlvl != 0)
		return;
	if((t = T - Timeinc) >= Timeinc)
		T = t;
	else if(T > Timeinc)
		T = Timeinc;
	else
		T *= 0.9;
}

static void
clearlines(void)
{
	int b, *bf;

	for(bf=bfield+nelem(bfield)-1, b=*bf; b!=0; b=*(--bf)){
		if(b != (1 << Ncol) - 1)
			continue;
		memmove(bfield+1, bfield, (bf-bfield) * sizeof *bf);
		memmove(playfield+Ncol, playfield, (bf-bfield) * Ncol);
		bf++;
		updatelevel();
	}
}

void
hold(void)
{
	int p;

	if(cur->flags & Fswapped)
		return;
	p = held;
	held = cur->type;
	spawn();
	if(p != -1)
		cur->type = p;
	cur->flags |= Fswapped;
}

static void
freeze(void)
{
	char *p;
	int n, x, y;
	u32int f;

	if((y = cur->y) >= Nrow)
		sysfatal("freeze: oob 1 piece type %d at %d,%d",
			cur->type, cur->x, cur->y);
	f = fours[cur->type][cur->rot];
	p = playfield + y * Ncol + cur->x;
	for(n=0, x=1<<(Nside*Nside-1); x>0; x>>=1, p++){
		if(f & x){
			if(y >= Nrow)
				sysfatal("freeze: oob 2 piece type %d %d,%d at %zd,%d",
					cur->type, cur->x, cur->y, p-(playfield+y*Ncol), y);
			if(*p != 0)
				sysfatal("freeze: overlapping piece type %d %d,%d at %zd,%d",
					cur->type, cur->x, cur->y, p-(playfield+y*Ncol), y);
			*p = cur->type + 1;
			bfield[y] |= 1 << (Ncol - 1 - cur->x - n);
		}
		if(++n == Nside){
			p += Ncol - Nside;
			n = 0;
			y++;
		}
	}
	clearlines();
	disengage();
	spawn();
}

void
drop(void)
{
	while(!collide(cur->x, cur->y+1, cur->rot))
		cur->y++;
	freeze();
}

void
gameover(void)
{
	print("you're done! finished!\n");
	quit();
}

void
step(void)
{
	if(cur == nil){
		spawn();
		return;
	}else if(collide(cur->x, cur->y+1, cur->rot)){
		freeze();
		return;
	}
	cur->y++;
}
