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
};
enum{
	Fswapped = 1<<0,
};
struct Current{
	int x;
	int y;
	int type;
	int rot;
	int flags;
	double thover;
};
extern Current *cur;
extern int fours[NF][Nrot];

enum{
	Nrow = 40,
	Nstartrow = 20,
	Nextrarows = 2,
	Ncol = 10,
	Block = 16,

	Wwidth = Block * Ncol,
	Wheight = Block * (Nstartrow + Nextrarows),

	K← = 1<<0,
	K→ = 1<<1,
	K↑ = 1<<2,
	K↓ = 1<<3,
	Kmove = K← | K→ | K↓,
	Krotl = 1<<4,
	Krotr = 1<<5,
	Khold = 1<<6,

	Tspeed0 = 1,	/* seconds */
};
#define T0	(double)BILLION / Tspeed0
extern char playfield[Ncol * Nrow];
extern double T;

enum{
	DOrange = 0xffff00ff,
	DPurple = 0xff00ffff,
};
