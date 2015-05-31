#include "world.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>

// Macro for disable unused warnings
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))

struct World {
	wsize_t x;
	wsize_t y;

	unsigned char limits;
	wsize_t minX;
	wsize_t maxX;

	struct Cell ***grid;
	struct list_head monitoredCells;
	unsigned int numMonCells;
};

struct Cell {
	struct list_head lh;

	wsize_t x;
	wsize_t y;
	char num_ref;
	bool alive;
};

// Auxiliary functions
static struct Cell *newCell(wsize_t x, wsize_t y, unsigned char num_ref,
	bool alive);
static struct Cell *_reviveCell(wsize_t x, wsize_t y, struct World *world);
static void addCell(struct Cell *cell, struct World *world);
static void deleteCell(struct Cell *cell, struct World *world);
static void addNeighbor(wsize_t x, wsize_t y, struct World *world);
static void rmNeighbor(wsize_t x, wsize_t y, struct World *world);
static void incRef(wsize_t x, wsize_t y, struct World *world);
void decRef(wsize_t x, wsize_t y, struct World *world);
static void checkLimits(wsize_t *x, wsize_t *y, const struct World *world);
static bool inLimits(wsize_t x, const struct World *world);
static void correctCoords(wsize_t *x, wsize_t *y, const struct World *world);


struct World *createWorld(wsize_t x, wsize_t y, unsigned char limits)
{
	struct World *world;
	struct Cell **grid;
	wsize_t minX, maxX;
	wsize_t i, j;

	// Define bounds
	minX = 0;
	maxX = x;

	if (limits) {
		minX = 1;
		maxX = x;
		x += 2;
	}

	// Allocate memory
	world = (struct World *) malloc(sizeof(struct World));
	world->grid = (struct Cell ***)malloc(x * sizeof(struct Cell *));
	grid = (struct Cell **)malloc(x * y * sizeof(struct Cell *));

	// Initialize pointers
	for (i = 0; i < x; ++i) {
		world->grid[i] = &grid[i*y];
		for (j = 0; j < y; ++j)
			world->grid[i][j] = NULL;
	}

	// Initialize struct
	world->x = x;
	world->y = y;
	world->limits = limits;
	world->minX = minX;
	world->maxX = maxX;
	INIT_LIST_HEAD(&world->monitoredCells);
	world->numMonCells = 0;

	return world;
}

inline void destroyWorld(struct World *world)
{
	struct Cell *cell, *tmp;

	list_for_each_entry_safe(cell, tmp, &world->monitoredCells, lh) {
		list_del(&cell->lh);
		free(cell);
	}
	free(world->grid[0]);
	free(world->grid);
	free(world);
}

inline void clearWorld(struct World *world)
{
	wsize_t i, j;

	for (i = 0; i < world->x; ++i) {
		for (j = 0; j < world->y; ++j) {
			if (world->grid[i][j] != NULL)
				deleteCell(world->grid[i][j], world);
		}
	}

	world->numMonCells = 0;
}

inline void getSize(wsize_t *x, wsize_t *y, const struct World *world)
{
	*x = world->x - 2*world->limits;
	*y = world->y - 2*world->limits;
}

void getRealSize(wsize_t *x, wsize_t *y, const struct World *world)
{
	*x = world->x;
	*y = world->y;
}

inline static struct Cell *newCell(wsize_t x, wsize_t y, unsigned char num_ref,
	bool alive)
{
	struct Cell *cell;

	cell = (struct Cell *)malloc(sizeof(struct Cell));
	cell->x = x;
	cell->y = y;
	cell->num_ref = num_ref;
	cell->alive = alive;

	return cell;
}

static void addCell(struct Cell *cell, struct World *world)
{
	list_add(&cell->lh, &world->monitoredCells);
	world->grid[cell->x][cell->y] = cell;
	++(world->numMonCells);
}

inline static void incRef(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;

	if (inLimits(x, world)) return;
	checkLimits(&x, &y, world);

	if (world->grid[x][y] != NULL)
		++(world->grid[x][y]->num_ref);
	else {
		cell = newCell(x, y, 1, false);
		addCell(cell, world);
	}
}

inline void decRef(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;

	if (inLimits(x, world)) return;
	checkLimits(&x, &y, world);

	cell = world->grid[x][y];
	if (cell == NULL) return;

	--(cell->num_ref);
	if (!cell->alive && cell->num_ref <= 0)
		deleteCell(cell, world);
}

static void addNeighbor(wsize_t x, wsize_t y, struct World *world)
{
	incRef(x, y-1, world);
	incRef(x, y+1, world);
	incRef(x-1, y, world);
	incRef(x-1, y-1, world);
	incRef(x-1, y+1, world);
	incRef(x+1, y, world);
	incRef(x+1, y-1, world);
	incRef(x+1, y+1, world);
}

static void rmNeighbor(wsize_t x, wsize_t y, struct World *world)
{
	decRef(x, y-1, world);
	decRef(x, y+1, world);
	decRef(x-1, y, world);
	decRef(x-1, y-1, world);
	decRef(x-1, y+1, world);
	decRef(x+1, y, world);
	decRef(x+1, y-1, world);
	decRef(x+1, y+1, world);
}

void reviveCell(wsize_t x, wsize_t y, struct World *world)
{
	correctCoords(&x, &y, world);
	checkLimits(&x, &y, world);
	_reviveCell(x, y, world);
}

inline struct Cell *_reviveCell(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;

	cell = world->grid[x][y];

	if (cell == NULL) {
		cell = newCell(x, y, 0, true);
		addCell(cell, world);
		addNeighbor(x, y, world);
	}
	else if (!cell->alive) {
		addNeighbor(x, y, world);
		cell->alive = true;
	}

	return cell;
}

void reviveCells(struct list_head *list, struct World *world)
{
	struct CellListNode *cln;

	list_for_each_entry(cln, list, lh)
		_reviveCell(cln->cell->x, cln->cell->y, world);
}

struct Cell *killCell(struct Cell *cell, struct World *world)
{
	wsize_t x = cell->x;
	wsize_t y = cell->y;

	/* checkLimits(&x, &y, world); */

	if (cell->alive) {
		rmNeighbor(x, y, world);
		if (cell->num_ref <= 0)
			deleteCell(cell, world);
		else
			cell->alive = false;
	}

	return cell;
}

void killCells(struct list_head *list, struct World *world)
{
	struct CellListNode *cln;

	list_for_each_entry(cln, list, lh)
		killCell(cln->cell, world);
}

static void deleteCell(struct Cell *cell, struct World *world)
{
	list_del(&cell->lh);
	world->grid[cell->x][cell->y] = NULL;
	free(cell);
	--(world->numMonCells);
}

inline static void checkLimits(wsize_t *x, wsize_t *y,
	const struct World *world)
{
	if      (*x < 0)         *x = world->x + *x;
	else if (*x >= world->x) *x = *x - world->x;

	if      (*y < 0)         *y = world->y + *y;
	else if (*y >= world->y) *y = *y - world->y;
}
inline static bool inLimits(wsize_t x, const struct World *world)
{
	return !world->limits || x <= 0 || x >= world->x-1;
}

inline static void correctCoords(wsize_t *x, wsize_t *y, const struct World *world)
{
	*x = *x + world->limits;
}

inline void getCellPos(wsize_t *x, wsize_t *y, const struct Cell *cell)
{
	*x = cell->x;
	*y = cell->y;
}

char dgetCellRefs(wsize_t x, wsize_t y, const struct World *world)
{
	correctCoords(&x, &y, world);
	checkLimits(&x, &y, world);
	return world->grid[x][y]->num_ref;
}

char getCellRefs(wsize_t x, wsize_t y, const struct World *world)
{
	return world->grid[x][y]->num_ref;
}

inline bool isCellAlive(const struct Cell *cell)
{
	return cell->alive;
}

inline bool isCellAlive_coord(wsize_t x, wsize_t y,
	const struct World *world)
{
	struct Cell *cell;

	checkLimits(&x, &y, world);
	cell = world->grid[x][y];

	return cell == NULL? false : cell->alive;
}

inline struct Cell *getCell(wsize_t x, wsize_t y, const struct World *world)
{
	correctCoords(&x, &y, world);
	checkLimits(&x, &y, world);
	return world->grid[x][y];
}

void addToList(struct Cell *cell, struct list_head *list)
{
	struct CellListNode *cellList;

	cellList = (struct CellListNode *)malloc(sizeof(struct CellListNode));
	cellList->cell = cell;
	list_add(&cellList->lh, list);
}

void addToList_coords(wsize_t x, wsize_t y, bool alive, struct list_head *list,
	struct World *world)
{
	struct CellListNode *cellList;

	checkLimits(&x, &y, world);

	cellList = (struct CellListNode *)malloc(sizeof(struct CellListNode));
	cellList->cell = newCell(x, y, 0, alive);
	list_add(&cellList->lh, list);
}

void freeList(struct list_head *list)
{
	struct CellListNode *cellList, *tmp;

	list_for_each_entry_safe(cellList, tmp, list, lh) {
		list_del(&cellList->lh);
		free(cellList);
	}
}

inline struct Cell *wit_first(const struct World *world)
{
	return list_entry(world->monitoredCells.next, struct Cell, lh);
}

inline struct Cell *wit_first_safe(const struct World *world, struct Cell **tmp)
{
	struct Cell *cell;

	cell = list_entry(world->monitoredCells.next, struct Cell, lh);
	*tmp = list_entry(cell->lh.next, struct Cell, lh);

	return cell;
}

struct Cell *wit_first_split(unsigned int *count, unsigned int indx,
	const struct World *world)
{
	struct list_head *cell;
	unsigned int i;

	*count = indx;
	cell = world->monitoredCells.next;
	for (i = 0; i < indx; ++i)
		cell = cell->next;

	return list_entry(cell, struct Cell, lh);
}

inline bool wit_done(const struct Cell *cell, const struct World *world)
{
	return &cell->lh != &world->monitoredCells;
}

inline bool wit_done_split(unsigned int count, const struct World *world)
{
	return count < world->numMonCells;
}

inline struct Cell *wit_next(const struct Cell *cell)
{
	return list_entry(cell->lh.next, struct Cell, lh);
}

inline struct Cell *wit_next_safe(struct Cell **tmp)
{
	struct Cell *cell;

	cell = *tmp;
	*tmp = list_entry((*tmp)->lh.next, struct Cell, lh);

	return cell;
}

inline struct Cell *wit_next_split(const struct Cell *cell, unsigned int *count,
	unsigned int splits)
{
	struct list_head *nextCell;
	unsigned int i;

	*count += splits;
	nextCell = cell->lh.next;
	for (i = 1; i < splits; ++i)
		nextCell = nextCell->next;

	return list_entry(nextCell, struct Cell, lh);
}
