#include "world.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>

// Macro for disable unused warnings
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))

struct World {
	wsize_t x;
	wsize_t y;

	struct Cell ***grid;
	struct list_head monitoredCells;
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
static void addCell(struct Cell *cell, struct World *world);
static void deleteCell(struct Cell *cell, struct World *world);
static void addNeighbor(wsize_t x, wsize_t y, struct World *world);
static void rmNeighbor(wsize_t x, wsize_t y, struct World *world);
static void checkLimits(wsize_t *x, wsize_t *y, const struct World *world);

struct World *createWorld(wsize_t x, wsize_t y)
{
	struct World *world;
	struct Cell **grid;
	wsize_t i, j;

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
	INIT_LIST_HEAD(&world->monitoredCells);

	return world;
}

inline void destroyWorld(struct World *world)
{
	struct Cell *cell, *tmp;

	list_for_each_entry_safe(cell, tmp, &world->monitoredCells, lh)
		deleteCell(cell, world);
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
}

inline void getSize(wsize_t *x, wsize_t *y, const struct World *world)
{
	*x = world->x;
	*y = world->y;

	return;
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
}

inline static void incRef(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;

	checkLimits(&x, &y, world);

	if (world->grid[x][y] != NULL)
		++(world->grid[x][y]->num_ref);
	else {
		cell = newCell(x, y, 1,false);
		addCell(cell, world);
	}
}

inline void decRef(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;

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

struct Cell *reviveCell(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;

	checkLimits(&x, &y, world);
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
		reviveCell(cln->cell->x, cln->cell->y, world);
}

struct Cell *killCell(struct Cell *cell, struct World *world)
{
	wsize_t x = cell->x;
	wsize_t y = cell->y;

	checkLimits(&x, &y, world);

	if (cell->alive) {
		rmNeighbor(x, y, world);
		cell->alive = false;
	}

	if (--(cell->num_ref) <= 0)
		deleteCell(cell, world);

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
	free(cell);
	world->grid[cell->x][cell->y] = NULL;
}

inline static void checkLimits(wsize_t *x, wsize_t *y,
	const struct World *world)
{
	if (*x < 0)               *x = world->x + *x;
	else if (*x >= world->x)  *x = *x - world->x;
	if (*y < 0)               *y = world->y + *y;
	else if (*y >= world->y)  *y = *y - world->y;
}

inline void getCellPos(wsize_t *x, wsize_t *y, const struct Cell *cell)
{
	*x = cell->x;
	*y = cell->y;
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

void freeList(struct list_head *list)
{
	struct CellListNode *cellList, *tmp;

	list_for_each_entry_safe(cellList, tmp, list, lh) {
		list_del(&cellList->lh);
		free(cellList);
	}
}

inline struct Cell *wit_first(struct World *world)
{
	return list_entry(world->monitoredCells.next, struct Cell, lh);
}

inline struct Cell *wit_first_safe(struct World *world, struct Cell **tmp)
{
	struct Cell *cell;

	cell = list_entry(world->monitoredCells.next, struct Cell, lh);
	*tmp = list_entry(cell->lh.next, struct Cell, lh);

	return cell;
}

inline bool wit_done(struct Cell *cell, struct World *world)
{
	return &cell->lh != &world->monitoredCells;
}

inline struct Cell *wit_next(struct Cell *cell)
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
