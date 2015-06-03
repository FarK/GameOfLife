#include "gol.h"
#include "world.h"
#include "list.h"
#include <stdlib.h>
#include <omp.h>

enum CellProcessing{
	GOL_REVIVE,
	GOL_KILL,
	GOL_SURVIVE,
	GOL_KEEP_DEAD
};

struct GOL {
	struct World *world;
	struct Stats *stats;

	struct list_head *toRevive;
	struct list_head *toKill;
	const struct Rule *rule;
	unsigned int numThreads;
};

static enum CellProcessing checkRule(struct Cell *cell,const struct Rule *rule);
static bool checkSubrule(unsigned char subrule, unsigned char aliveCounter);

struct GOL *golInit(unsigned int numThreads, const struct Rule *rule,
	struct World *world, struct Stats *stats)
{
	unsigned int i;
	struct GOL *gol;

	// Allocate memory
	gol = (struct GOL *)malloc(sizeof(struct GOL));
	gol->toRevive = (struct list_head *)
		malloc(numThreads * sizeof(struct list_head));
	gol->toKill = (struct list_head *)
		malloc(numThreads * sizeof(struct list_head));

	gol->rule = rule;
	gol->world = world;
	gol->numThreads = numThreads;
	gol->stats = stats;

	// Initialize lists
	for (i = 0; i < numThreads; ++i) {
		INIT_LIST_HEAD(&gol->toRevive[i]);
		INIT_LIST_HEAD(&gol->toKill[i]);
	}

	omp_set_num_threads(numThreads);

	return gol;
}

void golEnd(struct GOL *gol)
{
	free(gol->toRevive);
	free(gol->toKill);
	free(gol);
}

void iteration(struct GOL *gol)
{
	struct Cell *cell;
	unsigned int i;
	unsigned int count;
	unsigned int threadNum;
	double ccTime, wupTime, thTime;

	// TODO: it can be multithread?
	reviveCells(&gol->toRevive[0], gol->world);
	killCells(&gol->toKill[0], gol->world);

	ccTime = startMeasurement();
	#pragma omp parallel shared(gol) private(cell, count, threadNum, thTime)
	{
		thTime = startMeasurement();

		threadNum = omp_get_thread_num();

		for (cell = wit_first_split(&count, threadNum, gol->world);
		     wit_done_split(count, gol->world);
		     cell = wit_next_split(cell, &count, gol->numThreads))
		{
			switch (checkRule(cell, gol->rule)) {
			case GOL_REVIVE:
				addToList(cell, &gol->toRevive[threadNum]);
				break;
			case GOL_KILL:
				addToList(cell, &gol->toKill[threadNum]);
				break;
			case GOL_SURVIVE:
			case GOL_KEEP_DEAD:
			default:
				break;
			}
		}

		endMeasurement(thTime, threads[threadNum], gol->stats);
	}
	endMeasurement(ccTime, cellChecking, gol->stats);

	wupTime = startMeasurement();
	// Add lists
	for (i = 0; i < gol->numThreads; ++i) {
		reviveCells(&gol->toRevive[i], gol->world);
		freeList(&gol->toRevive[i]);
	}

	// Free lists
	for (i = 0; i < gol->numThreads; ++i) {
		killCells(&gol->toKill[i], gol->world);
		freeList(&gol->toKill[i]);
	}
	endMeasurement(wupTime, worldUpdate, gol->stats);
}

enum CellProcessing checkRule(struct Cell *cell, const struct Rule *rule)
{
	enum CellProcessing cProc;
	unsigned char aliveCounter = 0;
	bool satisfy;
	bool cellAlive;


	cellAlive = isCellAlive(cell);
	aliveCounter = getCellRefs(cell);

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

inline void gol_reviveCell(wsize_t x, wsize_t y, struct GOL *gol)
{
	addToList_coords(x, y, true, &gol->toRevive[0], gol->world);
}

inline void gol_killCell(wsize_t x, wsize_t y, struct GOL *gol)
{
	addToList_coords(x, y, false, &gol->toRevive[0], gol->world);
}
