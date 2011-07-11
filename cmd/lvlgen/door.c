#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include "lvlgen.h"

static int doorlocs(Lvl *lvl, Path *p, Loc ls[], int sz);
static void sortbydist(Loc ls[], int sz, Loc l0);
static double dist(Loc l0, Loc l1);
static void extdoor(Lvl *lvl, int x, int y, int z);
static int extdoorlocs(Lvl *lvl, Loc ls[], int sz);

/* This algorithm is technically not even guaranteed to ever
 * terminate, but its OK for now. */
Loc doorloc(Lvl *lvl, Path *p, Loc l0)
{
	int sz = lvl->w * lvl->h;
	Loc locs[sz];

	sz = doorlocs(lvl, p, locs, sz);
	assert(sz > 0);
	sortbydist(locs, sz, l0);

	int q = sz / 4;
	int min = 3 * q;
	int max = min + q;
	unsigned int i = max - min == 0 ? rnd(0, sz) : rnd(min, max);
	assert (i < sz);
	return locs[i];
}

static int doorlocs(Lvl *lvl, Path *p, Loc ls[], int sz)
{
	int i = 0;

	for (int x = 0; x < lvl->w; x++) {
	for (int y = 0; y < lvl->h - 1; y++) {
		Blkinfo bi = blkinfo(lvl, x, y+1, lvl->z);
		if (used(lvl, (Loc) {x, y}) && bi.flags & Tilecollide) {
			if (i == sz)
				fatal("Door loc array is too small");
			ls[i] = (Loc) {x, y};
			i++;
		}
	}
	}

	return i;
}

static Loc cmploc;

static int doorcmp(const void *a, const void *b)
{
	double da = dist(cmploc, *(Loc*) a);
	double db = dist(cmploc, *(Loc*) b);
	if (da > db)
		return 1;
	else if (da < db)
		return -1;
	return 0;
}

static void sortbydist(Loc ls[], int sz, Loc l0)
{
	cmploc = l0;
	qsort(ls, sz, sizeof(ls[0]), doorcmp);
}

static double dist(Loc l0, Loc l1)
{
	int dx = l1.x - l0.x;
	int dy = l1.y - l0.y;
	return sqrt(dx * dx + dy * dy);
}

enum { Nextra = 2 };

void extradoors(Rng *r, Lvl *lvl)
{
	int sz = lvl->w * lvl->h;
	Loc ls[sz];

	for (lvl->z = 0; lvl->z < lvl->d - 1; lvl->z++) {
		int n = extdoorlocs(lvl, ls, sz);
		for (int i = 0; i < Nextra && i < n; i++) {
			int j = rngintincl(r, 0, n);
			extdoor(lvl, ls[j].x, ls[j].y, lvl->z);
			ls[j] = ls[n-1];
			n--;
		}
	}
}

static void extdoor(Lvl *lvl, int x, int y, int z)
{
	Blkinfo bi0 = blkinfo(lvl, x, y, z);
	if (bi0.flags & Tilewater)
		blk(lvl, x, y, z)->tile = ')';
	else
		blk(lvl, x, y, z)->tile = '>';

	Blkinfo bi1 = blkinfo(lvl, x, y, z+1);
	if (bi1.flags & Tilewater)
		blk(lvl, x, y, z+1)->tile = '(';
	else
		blk(lvl, x, y, z+1)->tile = '<';
}

static int extdoorlocs(Lvl *lvl, Loc ls[], int sz)
{
	int i = 0;

	for (int x = 0; x < lvl->w; x++) {
	for (int y = 0; y < lvl->h - 1; y++) {
		if (blkinfo(lvl, x, y, lvl->z).flags & Tilereach
			&& blkinfo(lvl, x, y, lvl->z+1).flags & Tilereach
			&& blkinfo(lvl, x, y+1, lvl->z).flags & Tilecollide
			&& blkinfo(lvl, x, y+1, lvl->z+1).flags & Tilecollide) {
			if (i == sz)
				fatal("Door loc array is too small");
			ls[i] = (Loc) {x, y};
			i++;
		}
	}
	}

	return i;
}