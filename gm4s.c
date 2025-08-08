#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <keyboard.h>
#include "/sys/src/games/eui.h"
#include "dat.h"
#include "fns.h"

/* FIXME: games/4s has interesting rotation behavior, check out how it works;
 * it kind of implements some of the kicks in its way, check which behavior
 * makes more sense */
/* FIXME: pause button */

/* https://tetris.fandom.com/wiki/Playfield */

double T = T0;

enum{
	Trep0 = 120,
	Trep = 40,
};
static Channel *stepc, *evc;
static ulong oldkeys;

void
quit(void)
{
	threadexitsall(nil);
}

static void
kevent(ulong k)
{
	int moved;
	char r;
	static char rr[] = {Left, Up, Right, Down, Left, Up};

	if(cur == nil)
		return;
	if(k & Khold)
		hold();
	moved = 0;
	r = cur->rot;
	if(k & K↓ && !collide(cur->x, cur->y + 1, r)){
		cur->y++;
		moved = 1;
	}
	if(k & Krotl && !collide(cur->x, cur->y, (r = rr[1+cur->rot-1]))
	|| k & Krotr && !collide(cur->x, cur->y, (r = rr[1+cur->rot+1]))){
		cur->rot = r;
		moved = 1;
	}
	if(k & K← && !collide(cur->x - 1, cur->y, r)){
		cur->x--;
		moved = 1;
	}
	if(k & K→ && !collide(cur->x + 1, cur->y, r)){
		cur->x++;
		moved = 1;
	}
	if(k & K↑)
		drop();
	else if(cur->flags & Fhovering && moved)
		cur->lastmove = nanosec();
}

static void
pollproc(void *)
{
	ulong k, ke;
	u64int t, trep;

	trep = 0;
	for(;;){
		ke = keys;
		if(ke == 0)
			goto next;
		k = ke ^ ke & oldkeys;
		t = nanosec() / MILLION;
		if(k == 0){
			if((ke & Ktriggers) == 0 && ke & Kmove){
				if(t >= trep){
					k = ke & Kmove;
					if(send(evc, &k) < 0)
						return;
					trep = t + Trep;
				}
			}
		}else{
			if(send(evc, &k) < 0)
				return;
			trep = t + Trep0;
		}
	next:
		oldkeys = ke;
		sleep(1);
	}
}

static void
ticproc(void *)
{
	double t0;
	vlong t, Δt;

	t0 = nanosec();
	for(;;){
		nbsendul(stepc, 1);
		t = nanosec();
		Δt = t - t0;
		t0 += T * (1 + Δt / T);
		if(Δt < T)
			sleep((T - Δt) / MILLION);
	}
}

static void
usage(void)
{
	sysfatal("usage: %s [-b STR]", argv0);
}

void
threadmain(int argc, char **argv)
{
	ulong k;

	ARGBEGIN{
	case 'b':
		readboard(EARGF(usage()));
		break;
	}ARGEND
	if((stepc = chancreate(sizeof(ulong), 1)) == nil
	|| (evc = chancreate(sizeof(ulong), 0)) == nil)
		sysfatal("chancreate: %r");
	initemu(Vwidth, Vheight, 4, XRGB32, 1, nil);
	fmtinstall('H', encodefmt);
	regkey("up", Kup, K↑);
	regkey("down", Kdown, K↓);
	regkey("left", Kleft, K←);
	regkey("right", Kright, K→);
	regkey("b", 'z', Krotl);
	regkey("a", 'x', Krotr);
	regkey("l1", ' ', Khold);
	if(proccreate(pollproc, nil, 4096) < 0)
		sysfatal("proccreate: %r");
	if(proccreate(ticproc, nil, 4096) < 0)
		sysfatal("proccreate: %r");
	srand(time(nil));
	initgame();
	enum{
		Astep,
		Akey,
	};
	Alt a[] = {
		[Astep] {stepc, nil, CHANRCV},
		[Akey] {evc, &k, CHANRCV},
		{nil, nil, CHANEND}
	};
	for(;;){
		switch(alt(a)){
		default: threadexitsall(nil);
		case Astep:
			step();
			break;
		case Akey:
			kevent(k);
			break;
		}
		redraw();
	}
}
