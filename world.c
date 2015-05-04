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
	unsigned char num_ref;
	unsigned char state;
};

// Auxiliary functions
static struct Cell *newCell(wsize_t x, wsize_t y, unsigned char num_ref,
	unsigned char state);
static void addCell(struct Cell *cell, struct World *world);
static void addAliveCell(struct Cell *cell, struct World *world);
static void addNewDeadCell(wsize_t x, wsize_t y, struct World *world);
static void deleteCell(struct Cell *cell, struct World *world);
static void rmDeadCell(wsize_t x, wsize_t y, struct World *world);
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
	unsigned char state)
{
	struct Cell *cell;

	cell = (struct Cell *)malloc(sizeof(struct Cell));
	cell->x = x;
	cell->y = y;
	cell->num_ref = num_ref;
	cell->state = state;

	return cell;
}

static void addCell(struct Cell *cell, struct World *world)
{
	list_add(&cell->lh, &world->monitoredCells);
	world->grid[cell->x][cell->y] = cell;
}

inline static void addNewDeadCell(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;

	checkLimits(&x, &y, world);

	if (world->grid[x][y] != NULL) {
		++(world->grid[x][y]->num_ref);
		cell = world->grid[x][y];
	}
	else {
		cell = newCell(x, y, 1, CS_NEW | CS_DEAD);
		addCell(cell, world);
	}
}

static void addNeighbor(wsize_t x, wsize_t y, struct World *world)
{
	addNewDeadCell(x, y-1, world);
	addNewDeadCell(x, y+1, world);
	addNewDeadCell(x-1, y, world);
	addNewDeadCell(x-1, y-1, world);
	addNewDeadCell(x-1, y+1, world);
	addNewDeadCell(x+1, y, world);
	addNewDeadCell(x+1, y-1, world);
	addNewDeadCell(x+1, y+1, world);
}

static void rmNeighbor(wsize_t x, wsize_t y, struct World *world)
{
	rmDeadCell(x, y-1, world);
	rmDeadCell(x, y+1, world);
	rmDeadCell(x-1, y, world);
	rmDeadCell(x-1, y-1, world);
	rmDeadCell(x-1, y+1, world);
	rmDeadCell(x+1, y, world);
	rmDeadCell(x+1, y-1, world);
	rmDeadCell(x+1, y+1, world);
}

inline static void addAliveCell(struct Cell *cell, struct World *world)
{
	wsize_t x = cell->x;
	wsize_t y = cell->y;

	addCell(cell, world);
	addNeighbor(x, y, world);
}

struct Cell *addNewAliveCell(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;

	checkLimits(&x, &y, world);

	if (world->grid[x][y] == NULL) {
		cell = newCell(x, y, 0, CS_NEW | CS_ALIVE);
		addAliveCell(cell, world);
	}
	else
		cell = world->grid[x][y];

	return cell;
}

inline void rmDeadCell(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;

	checkLimits(&x, &y, world);
	cell = world->grid[x][y];

	if (cell != NULL && --(cell->num_ref) == 0)
		deleteCell(cell, world);
}

struct Cell *rmCell(struct Cell *cell, struct World *world)
{
	wsize_t x = cell->x;
	wsize_t y = cell->y;

	if (cell->state & CS_ALIVE) {
		deleteCell(cell, world);
		rmNeighbor(x, y, world);
	}
	else if (cell->state & CS_DEAD) {
		rmDeadCell(cell->x, cell->y, world);
	}

	return cell;
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

inline void reviveCell(struct Cell *cell, struct World *world)
{
	cell->state = CS_NEW | CS_ALIVE;
	addNeighbor(cell->x, cell->y, world);
}

inline void killCell(struct Cell *cell, struct World *world)
{
	cell->state = CS_NEW | CS_DEAD;
	rmNeighbor(cell->x, cell->y, world);
}

inline unsigned char getCellState(const struct Cell *cell)
{
	return cell->state;
}

inline struct Cell *getCell(wsize_t x, wsize_t y, const struct World *world)
{
	checkLimits(&x, &y, world);
	return world->grid[x][y];
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
