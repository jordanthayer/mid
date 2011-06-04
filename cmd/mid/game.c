#include "../../include/mid.h"
#include "../../include/log.h"
#include <stdlib.h>
#include <stdbool.h>
#include "game.h"
#include "resrc.h"

struct Game {
	Point transl;
	Lvl *lvl;
	Player *player;
	Inv inv;
	Enemy *e;
};

Game *gamenew(void)
{
	Game *gm = calloc(1, sizeof(*gm));
	if (!gm)
		return NULL;

	gm->lvl = resrcacq(lvls, "lvl/0.lvl", NULL);
	if (!gm->lvl)
		fatal("Failed to load level lvl/0.lvl: %s", miderrstr());
	gm->player = playernew(64, 96);
	gm->e = enemynew('u', (Point){128,96});

	// Testing items
	Item *axe = itemnew("Golden Pickaxe", "gaxe/anim");
	invmod(&gm->inv, axe, 0, 0);
	axe = itemnew("Golden Pickaxe", "gaxe/anim");
	invmod(&gm->inv, axe, 1, 0);
	axe = itemnew("Golden Pickaxe", "gaxe/anim");
	invmod(&gm->inv, axe, 1, 1);

	return gm;
}

void gamefree(Scrn *s)
{
	Game *gm = s->data;
	playerfree(gm->player);
	gm->e->mt->free(gm->e);
	resrcrel(lvls, "lvl/0.lvl", NULL);
	free(gm);
}

void gameupdate(Scrn *s, Scrnstk *stk)
{
	Game *gm = s->data;
	lvlupdate(anim, gm->lvl);
	playerupdate(gm->player, gm->lvl, &gm->transl);
	gm->e->mt->update(gm->e, gm->player, gm->lvl);
}

void gamedraw(Scrn *s, Gfx *g)
{
	Game *gm = s->data;
	gfxclear(g, (Color){ 0, 0, 0, 0 });
	lvldraw(g, anim, gm->lvl, true, gm->transl);
	playerdraw(g, gm->player, gm->transl);
	gm->e->mt->draw(gm->e, g, gm->transl);
	lvldraw(g, anim, gm->lvl, false, gm->transl);
	gfxflip(g);
}

void gamehandle(Scrn *s, Scrnstk *stk, Event *e)
{
	if(e->type != Keychng || e->repeat)
		return;

	Game *gm = s->data;

	if(e->down && e->key == kmap[Mvinv]){
		scrnstkpush(stk, invscrnnew(&gm->inv, gm->lvl, playerpos(gm->player)));
		return;
	}

	playerhandle(gm->player, e);
}

Scrnmt gamemt = {
	gameupdate,
	gamedraw,
	gamehandle,
	gamefree,
};

