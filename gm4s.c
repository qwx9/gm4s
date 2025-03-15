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
	char r;
	static char rr[] = {Left, Up, Right, Down, Left, Up};

	if(cur == nil)
		return;
	if(k & Khold)
		hold();
	if(k & K↓ && !collide(cur->x, cur->y + 1, cur->rot))
		cur->y++;
	if(k & Krotl && !collide(cur->x, cur->y, (r = rr[1+cur->rot-1]))
	|| k & Krotr && !collide(cur->x, cur->y, (r = rr[1+cur->rot+1])))
		cur->rot = r;
	if(k & K← && !collide(cur->x - 1, cur->y, cur->rot))
		cur->x--;
	if(k & K→ && !collide(cur->x + 1, cur->y, cur->rot))
		cur->x++;
	if(k & K↑)
		drop();
}

static void
pollproc(void *)
{
	int r;
	ulong k, ke, km, old;
	u64int t, t0;

	for(old=0;;){
		if(recv(keychan, &ke) < 0)
			return;
		if(ke == 0){
			old = keys & Ktriggers;
			continue;
		}
		k = ke ^ ke & old;
		old = ke & Ktriggers;
		if(k == 0)
			continue;
		if(send(evc, &k) < 0)
			return;
		t0 = nanosec() / MILLION + 125;
		for(k&=Kmove, km=k; k&Kmove; k&=Kmove){
			if((r = nbrecv(keychan, &ke)) < 0)
				return;
			else if(r == 0){
				if((t = nanosec() / MILLION) >= t0){
					if(send(evc, &k) < 0)
						return;
					t0 = t + 50;
				}
				old = keys & Ktriggers;
				sleep(1);
				continue;
			}
			if(ke == 0){
				old = keys & Ktriggers;
				break;
			}
			k = ke ^ ke & old;
			old = ke & Ktriggers;
			if(k == km)
				continue;
			if(send(evc, &k) < 0)
				return;
			t0 = nanosec() / MILLION + 50;
		}
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
