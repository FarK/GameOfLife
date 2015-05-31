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

static struct list_head *toRevive;
static struct list_head *toKill;

static enum CellProcessing checkRule(wsize_t x, wsize_t y,
	const struct Rule *rule, const struct World *world);
static bool checkSubrule(unsigned char subrule, unsigned char aliveCounter);

void golInit(unsigned int numThreads)
{
	unsigned int i;

	omp_set_num_threads(numThreads);

	// Allocate memory
	toRevive = (struct list_head *)
		malloc(numThreads * sizeof(struct list_head));
	toKill = (struct list_head *)
		malloc(numThreads * sizeof(struct list_head));

	// Initialize lists
	for (i = 0; i < numThreads; ++i) {
		INIT_LIST_HEAD(&toRevive[i]);
		INIT_LIST_HEAD(&toKill[i]);
	}
}

void golEnd()
{
	free(toRevive);
	free(toKill);
}

void iteration(struct World *world, const struct Rule *rule,
	struct BoundaryCells **bcells)
{
	struct Cell *cell;
	unsigned int i;
	unsigned int count;
	unsigned int threadNum, numThreads;
	wsize_t x, y;
	enum WorldBound bound;
	int indx;

	// TODO: it can be multithread?
	reviveCells(&toRevive[0], world);
	killCells(&toKill[0], world);

	#pragma omp parallel shared(toRevive, toKill, world, numThreads, rule)\
	                     private(cell, count, threadNum, x, y)
	{
		numThreads = omp_get_num_threads();
		threadNum = omp_get_thread_num();

		for (cell = wit_first_split(&count, threadNum, world);
		     wit_done_split(count, world);
		     cell = wit_next_split(cell, &count, numThreads))
		{
			getCellPos(&x, &y, cell);

			switch (checkRule(x, y, rule, world)) {
				case GOL_REVIVE:
					addToList(cell, &toRevive[threadNum]);

					bound = boundsCheck(&x, &y, world);
					if (bound != WB_NONE) {
						addToRevive(x, y, bcells[bound]);
					}
					break;
				case GOL_KILL:
					addToList(cell, &toKill[threadNum]);

					bound = boundsCheck(&x, &y, world);
					if (bound != WB_NONE) {
						addToKill(x, y, bcells[bound]);
					}
					break;
				case GOL_SURVIVE:
				case GOL_KEEP_DEAD:
				default:
					break;
			}
		}
	}

	// Add lists
	for (i = 0; i < numThreads; ++i) {
		reviveCells(&toRevive[i], world);
		freeList(&toRevive[i]);
	}

	// Free lists
	for (i = 0; i < numThreads; ++i) {
		killCells(&toKill[i], world);
		freeList(&toKill[i]);
	}
}

enum CellProcessing checkRule(wsize_t x, wsize_t y, const struct Rule *rule,
	const struct World *world)
{
	enum CellProcessing cProc;
	unsigned char aliveCounter = 0;
	bool satisfy;
	bool cellAlive;


	cellAlive = isCellAlive_coord(x, y, world);
	aliveCounter = getCellRefs(x, y, world);

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

inline void gol_reviveCell(wsize_t x, wsize_t y, struct World *world)
{
	addToList_coords(x, y, true, &toRevive[0], world);
}

inline void gol_killCell(wsize_t x, wsize_t y, struct World *world)
{
	addToList_coords(x, y, false, &toRevive[0], world);
}
