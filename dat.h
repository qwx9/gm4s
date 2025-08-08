typedef struct Current Current;
enum{
	FI,
	FZ,
	FS,
	FJ,
	FL,
	FO,
	FT,
	NF,

	Up = 0,
	Right,
	Down,
	Left,
	Nrot,

	Nside = 4,

	Ntest = 5,	/* SRS */
};
enum{
	Fswapped = 1<<0,
	Fhovering = 1<<1,
};
struct Current{
	int x;
	int y;
	int type;
	int rot;
	int flags;
	u64int lastmove;
};
extern Current *cur;
extern int fours[NF][Nrot];
extern rkick[NF][Nrot][Ntest*2], lkick[NF][Nrot][Ntest*2];

enum{
	Nrow = 40,
	Nstartrow = 20,
	Nextrarows = 2,
	Ncol = 10,
	Block = 16,

	Wwidth = Ncol,
	Wheight = Nstartrow + Nextrarows,
	Wside = 3,
	Vwidth = Block * (Wwidth + 2 * Wside),
	Vheight = Block * Wheight,
};
extern u32int palette[256];
extern uchar sprites[NF][Block*Block];

enum{
	K← = 1<<0,
	K→ = 1<<1,
	K↑ = 1<<2,
	K↓ = 1<<3,
	Kmove = K← | K→ | K↓,
	Krotl = 1<<4,
	Krotr = 1<<5,
	Khold = 1<<6,
	Ktriggers = K↑ | Krotl | Krotr | Khold,

	Tspeed0 = 1,	/* seconds */
};
#define T0	(double)BILLION / Tspeed0
extern char playfield[Ncol * Nrow];
extern int held;
extern int next[4];
extern double T;

enum{
	DOrange = 0xff7f00ff,
	DPurple = 0xff00ffff,
};
