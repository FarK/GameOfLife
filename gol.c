#include "gol.h"
#include "world.h"
#include "list.h"
#include <stdlib.h>

enum CellProcessing{
	GOL_REVIVE,
	GOL_KILL,
	GOL_SURVIVE,
	GOL_KEEP_DEAD
};

static enum CellProcessing checkRule(wsize_t x, wsize_t y,
	const struct Rule *rule, const struct World *world);
static bool checkSubrule(unsigned char subrule, unsigned char aliveCounter);

void iteration(struct World *world, const struct Rule *rule)
{
	struct list_head toRevive, toKill;

	struct Cell *cell, *tmp = NULL;
	wsize_t x, y;

	INIT_LIST_HEAD(&toRevive);
	INIT_LIST_HEAD(&toKill);

	for (cell = wit_first_safe(world, &tmp);
	     wit_done(cell, world);
	     cell = wit_next_safe(&tmp))
	{
		getCellPos(&x, &y, cell);

		switch (checkRule(x, y, rule, world)) {
			case GOL_REVIVE:
				addToList(cell, &toRevive);
				break;
			case GOL_KILL:
				addToList(cell, &toKill);
				break;
			case GOL_SURVIVE:
			case GOL_KEEP_DEAD:
			default:
				break;
		}
	}

	reviveCells(&toRevive, world);
	killCells(&toKill, world);
	freeList(&toRevive);
	freeList(&toKill);
}

enum CellProcessing checkRule(wsize_t x, wsize_t y, const struct Rule *rule,
	const struct World *world)
{
	enum CellProcessing cProc;
	unsigned char aliveCounter = 0;
	bool satisfy;
	bool cellAlive;


	cellAlive = isCellAlive_coord(x, y, world);

	if (isCellAlive_coord(x,   y-1, world)) ++aliveCounter;
	if (isCellAlive_coord(x,   y+1, world)) ++aliveCounter;
	if (isCellAlive_coord(x-1, y,   world)) ++aliveCounter;
	if (isCellAlive_coord(x-1, y-1, world)) ++aliveCounter;
	if (isCellAlive_coord(x-1, y+1, world)) ++aliveCounter;
	if (isCellAlive_coord(x+1, y,   world)) ++aliveCounter;
	if (isCellAlive_coord(x+1, y-1, world)) ++aliveCounter;
	if (isCellAlive_coord(x+1, y+1, world)) ++aliveCounter;

	if (cellAlive) {
		satisfy = checkSubrule(rule->survive, aliveCounter);

		if (satisfy)
			cProc = GOL_SURVIVE;
		else
			cProc = GOL_KILL;
	}
	else{
		satisfy = checkSubrule(rule->birth, aliveCounter);

		if (satisfy)
			cProc = GOL_REVIVE;
		else
			cProc = GOL_KEEP_DEAD;
	}

	return cProc;
}

inline static bool checkSubrule(unsigned char subrule,
	unsigned char aliveCounter)
{
	int i;
	unsigned char ruleIndx;
	bool satisfy = false;

	for (i = 1, ruleIndx = 1;
	     ruleIndx < (1<<7) && !satisfy ;
	     ++i, ruleIndx <<= 1)
	{
		if (subrule & ruleIndx)
			satisfy |= aliveCounter == i;
	}

	return satisfy;
}
