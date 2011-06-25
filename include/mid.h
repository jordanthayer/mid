#include <stdio.h> // FILE

/* Mean frame time.  May be useful for comparing computation effort. */
extern double meanftime;
extern int debugging;

void *xalloc(unsigned long n, unsigned long sz);
void xfree(void*);

const char *miderrstr(void);
void seterrstr(const char *fmt, ...);

typedef struct Point Point;
struct Point{
	double x, y;
};

Point vecadd(Point, Point);

typedef struct Line1d Line1d;
struct Line1d{
	double a, b;
};

/*
Returns the absolute value of the intersection of two lines.
Returns a negative number if the lines don't intersect.
*/
double isection1d(Line1d, Line1d);

typedef struct Rect Rect;
struct Rect{
	Point a, b;
};

Line1d rectprojx(Rect);
Line1d rectprojy(Rect);
void rectmv(Rect *, double dx, double dy);
/* Makes point a the min,min and point b the max,max. */
Rect rectnorm(Rect r);
void ptmv(Point *, double dx, double dy);

_Bool rectcontains(Rect, Point);

typedef struct Isect Isect;
struct Isect{
	_Bool is;
	double dx, dy; /* isection1d of x and y projs */
};

Isect isection(Rect, Rect);
int isect(Rect, Rect);
double isectarea(Isect is);

/*
The minimal intersection will be positive, and the maximum intersection will
be negative. If both intersections are equal, both dx and dy will be positive.
*/
Isect minisect(Rect, Rect);


typedef struct Color Color;
struct Color{
	unsigned char r, g, b, a;
};

typedef struct Gfx Gfx;

Gfx *gfxinit(int w, int h, const char *title);
void gfxfree(Gfx *);
Point gfxdims(const Gfx *);
void gfxflip(Gfx *);
void gfxclear(Gfx *, Color);
void gfxdrawpoint(Gfx *, Point, Color);
void gfxfillrect(Gfx *, Rect, Color);
void gfxdrawrect(Gfx *, Rect, Color);

typedef struct Img Img;

Img *imgnew(const char *path);
void imgfree(Img *);
/* Returns negative dimensions on failure. */
Point imgdims(const Img *);
void imgdraw(Gfx *, Img *, Point);

typedef struct Txt Txt;

Txt *txtnew(const char *font, int sz, Color);
void txtfree(Txt *);
Point txtdims(const Txt *, const char *fmt, ...);
Img *txt2img(Gfx *, Txt *, const char *fmt, ...);
// Prefer txt2img to this for static text
Point txtdraw(Gfx *, Txt *, Point, const char *fmt, ...);

_Bool sndinit(void);
void sndfree(void);

typedef struct Music Music;

Music *musicnew(const char *);
void musicfree(Music *);
void musicstart(Music *, int fadein);
void musicstop(int fadeout);
void musicpause(void);
void musicresume(void);

typedef struct Sfx Sfx;

Sfx *sfxnew(const char *);
void sfxfree(Sfx *);
void sfxplay(Sfx *);

enum Eventty{
	Quit,
	Keychng,
	Mousemv,
	Mousebt,
};

enum{
	Mouse1 = 1,
	Mouse2 = Mouse1 << 1,
	Mouse3 = Mouse2 << 1,
};

typedef struct Event Event;
struct Event{
	enum Eventty type;

	_Bool down;
	_Bool repeat;
	char key;

	double x, y, dx, dy;
	int butt;
};

_Bool pollevent(Event *);

typedef struct Scrn Scrn;
typedef struct Scrnmt Scrnmt;
typedef struct Scrnstk Scrnstk;

struct Scrn{
	Scrnmt *mt;
	void *data;
};

enum { Ticktm = 20 /* ms */ };

struct Scrnmt{
	void (*update)(Scrn *, Scrnstk *);
	void (*draw)(Scrn *, Gfx *);
	void (*handle)(Scrn *, Scrnstk *, Event *);
	void (*free)(Scrn *);
};

Scrnstk *scrnstknew(void);
void scrnstkfree(Scrnstk *);
/* Stack now owns Scrn, will call scrn->mt->free(scrn) when popped. */
void scrnstkpush(Scrnstk *, Scrn *);
Scrn *scrnstktop(Scrnstk *);
void scrnstkpop(Scrnstk *);

void scrnrun(Scrnstk *, Gfx *);

typedef struct Rtab Rtab;

unsigned int strhash(const char *);

typedef struct Resrcops Resrcops;
struct Resrcops {
	void*(*load)(const char *path, void *aux);
	void(*unload)(const char *path, void *resrc, void *aux); /* may be NULL */
	unsigned int (*hash)(const char *path, void *aux); /* may be NULL */
	_Bool (*eq)(void *aux0, void *aux1); /* may be NULL */
};

Rtab *rtabnew(Resrcops *);
/* unloads all resources and frees the table. */
void rtabfree(Rtab *);
/* Acquire a reference to a resource (loading it if necessary).  The
 * 3rd param is passed as aux data as the 2nd param of load. */
void *resrcacq(Rtab *, const char *file, void *aux);
/* Release a reference to a resource. */
void resrcrel(Rtab *, const char *file, void *aux);

typedef struct Txtinfo Txtinfo;
struct Txtinfo {
	unsigned int size;
	Color color;
};

extern Rtab *imgs;
extern Rtab *anims;
extern Rtab *txt;
extern Rtab *music;
extern Rtab *sfx;
void initresrc(void);
void freeresrc(void);

typedef struct Anim Anim;
Anim *animnew(const char *);
void animfree(Anim *);
void animupdate(Anim *, int);
void animdraw(Gfx *, Anim *, Point);
void animreset(Anim *a);

typedef struct Blk Blk;
struct Blk {
	char tile;
	char flags;
};

typedef struct Lvl Lvl;
struct Lvl {
	int d, w, h, z;
	Blk blks[];
};

Lvl *lvlnew(int, int, int);
Lvl *lvlread(FILE *);
void lvlwrite(FILE *, Lvl *);
void lvlfree(Lvl *);
_Bool lvlinit();
void lvlupdate(Lvl *l);
void lvldraw(Gfx *g, Lvl *l, _Bool bkgrnd, Point offs);
void lvlminidraw(Gfx *g, Lvl *l, Point offs, int scale);
/* Returns the reverse vector that must be added to v in order to
 * respect collisions. */
Isect lvlisect(Lvl *l, Rect r, Point v);

typedef struct Blkinfo Blkinfo;
struct Blkinfo {
	int x, y, z;
	unsigned int flags;
};

enum {
	Tilecollide = 1<<0,
	Tilereach = 1<<1,
	Tilewater = 1<<2,
	Tilefdoor = 1<<3,
	Tilebdoor = 1<<4,
};

void lvluseshrine(Lvl *, int x, int y);

enum { Theight = 32, Twidth = 32 };

Blkinfo blkinfo(Lvl *l, int x, int y);
/* Get the information on the dominant block that r is overlapping. */
Blkinfo lvlmajorblk(Lvl *l, Rect r);

static inline Blk *blk(Lvl *l, int x, int y, int z)
{
	return &l->blks[z * l->w * l->h + y * l->w + x];
}


float blkgrav(int flags);
float blkdrag(int flags);

/* Update the visibility of the level given that the player is viewing
 * the level from location (x, y). */
void lvlvis(Lvl *l, int x, int y);

enum Action{
	Mvleft,
	Mvright,
	Mvact,
	Mvjump,
	Mvinv,
	Mvsword,
	Nactions,
};

_Bool keymapread(char km[Nactions], char *fname);
extern char kmap[Nactions];

enum { Scrnw = 1024, Scrnh = 576 };

/* Buffer from side of screen at which to begin scrolling. */
enum { Scrlbuf = 384 };

enum { Tall = 32, Wide = 32 };

enum { Maxdy = 12 };

typedef struct Body Body;
struct Body {
	Rect bbox;
	Point vel;
	Point a;
	_Bool fall;
};

void bodyinit(Body *, int x, int y);
void bodyupdate(Body *b, Lvl *l);

typedef struct Sword Sword;
struct Sword{
	Img *img[2];
	Rect loc[2];
	int cur;

	int pow;
};

void sworddraw(Gfx*, Sword*, Point tr);

typedef enum Act Act;
enum Act {
	Stand,
	Walk,
	Jump,
	Nacts
};

typedef struct Item Item;
enum { Maxinv = 15 };

typedef struct Player Player;
struct Player {
	Anim *leftas[Nacts];
	Anim *rightas[Nacts];
	Anim **anim;
	Act act;
	Point imgloc;

	Body body;

	_Bool acting;
	_Bool statup;

	int jframes;
	int iframes; // invulnerability after damage;
	int sframes;

	/* if changed, update visibility. */
	Blkinfo bi;

	int hp;
	int dex;

	int curhp;
	int money;
	Item *inv[Maxinv];

	Sword sw;
};

void playerinit(Player *p, int x, int y);
void playerupdate(Player *, Lvl *l, Point *tr);
void playerdraw(Gfx *, Player *, Point tr);
void playerhandle(Player *, Event *);
Point playerpos(Player *);
Rect playerbox(Player *);
void playerdmg(Player *, int x);
_Bool playertake(Player *, Item *);

typedef struct Enemy Enemy;
typedef struct Enemymt Enemymt;

struct Enemymt{
	void (*free)(Enemy*);
	void (*update)(Enemy*, Player*, Lvl*);
	void (*draw)(Enemy*, Gfx*, Point tr);
};

struct Enemy{
	Enemymt *mt;
	Body b;
	int hp;
	void *data;
};

_Bool enemyinit(Enemy *, unsigned char id, int x, int y);

typedef enum ItemID ItemID;
enum ItemID{
	ItemNone,
	ItemStatup,
	ItemCopper,
	ItemMax
};

struct Item{
	ItemID id;
	Body bod;
	_Bool gotit;
};

_Bool iteminit(Item*, ItemID id, Point p);
void itemupdateanims(void);
void itemupdate(Item*, Player*, Lvl*);
void itemdraw(Item*, Gfx*, Point tr);
void iteminvdraw(Item*, Gfx*, Point p);
char *itemname(Item*);

typedef enum EnvID EnvID;
enum EnvID{
	EnvNone,
	EnvShrempty,
	EnvShrused,
	EnvMax
};

typedef struct Env Env;
struct Env{
	EnvID id;
	Body body;
	_Bool gotit;
};

_Bool envinit(Env*, EnvID, Point);
void envupdateanims(void);
void envupdate(Env*, Lvl*);
void envdraw(Env*,  Gfx*, Point tr);
void envact(Env*, Player*, Lvl*);

enum {
	Maxenms = 32,
	Maxitms = 32,
	Maxenvs = 16,
	Maxz = 5,
};

typedef struct Zone Zone;
struct Zone {
	Lvl *lvl;
	Item itms[Maxz][Maxitms];
	Env envs[Maxz][Maxenvs];
	Enemy enms[Maxz][Maxenms];
};

Zone *zoneread(FILE *);
void zonewrite(FILE *, Zone *z);
void zonefree(Zone *z);


_Bool scanbuf(char *buf, char *fmt, ...);
_Bool printbuf(char *buf, size_t sz, char *fmt, ...);
