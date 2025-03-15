#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <keyboard.h>
#include "/sys/src/games/eui.h"
#include "dat.h"
#include "fns.h"

extern Channel *keychan;

/* https://tetris.fandom.com/wiki/Playfield */

double T = T0;

static Channel *stepc, *evc;

void
quit(void)
{
	threadexitsall(nil);
}

static void
kevent(ulong k)
{
	char r, rot;
	static char rr[] = {Left, Up, Right, Down, Left, Up};

	if(cur == nil)
		return;
	if(k & Khold)
		hold();
	rot = cur->rot;
	if(k & Krotl && !collide(cur->x, cur->y, (r = rr[1+rot-1]))
	|| k & Krotr && !collide(cur->x, cur->y, (r = rr[1+rot+1])))
		cur->rot = r;
	if(k & K← && !collide(cur->x - 1, cur->y, rot))
		cur->x--;
	if(k & K→ && !collide(cur->x + 1, cur->y, rot))
		cur->x++;
	if(k & K↑)
		drop();
	if(k & K↓ && !collide(cur->x, cur->y + 1, rot))
		cur->y++;
}

static void
pollproc(void *)
{
	int dt;
	ulong k, ke, old;

	for(old=0;;){
		if(recv(keychan, &ke) < 0)
			return;
		k = ke ^ ke & old;
		old = ke & Ktriggers;
		if(k == 0)
			continue;
		if(send(evc, &k) < 0)
			return;
		for(dt=125, k&=Kmove; k&Kmove; k&=Kmove){
			sleep(dt);
			switch(nbrecv(keychan, &ke)){
			case -1: return;
			case 1: k = ke ^ ke & old; old = ke & Ktriggers; break;
			}
			if(send(evc, &k) < 0)
				return;
			dt = 40;
		}
	}
}

static void
ticproc(void *)
{
	double t0;
	vlong t, Δt;

	t0 = nsec();
	for(;;){
		nbsendul(stepc, 1);
		t = nsec();
		Δt = t - t0;
		t0 += T * (1 + Δt / T);
		if(Δt < T)
			sleep((T - Δt) / MILLION);
	}
}

void
disengage(void)
{
	sendul(keychan, 0);
}

void
threadmain(int argc, char **argv)
{
	ulong k;

	ARGBEGIN{
	}ARGEND
	if((stepc = chancreate(sizeof(ulong), 1)) == nil
	|| (evc = chancreate(sizeof(ulong), 0)) == nil
	|| (keychan = chancreate(sizeof(ulong), 8)) == nil)
		sysfatal("chancreate: %r");
	/* FIXME: simultaneous kproc and joyproc in eui */
	initemu(Wwidth, Wheight, 4, XRGB32, 1, nil);
	initimg();
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
