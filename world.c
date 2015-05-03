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

struct Cell *addNewCell(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;

	if (world->grid[x][y] == NULL) {
		cell = (struct Cell *)malloc(sizeof(struct Cell));
		cell->x = x;
		cell->y = y;

		world->grid[x][y] = cell;
		addCell(cell, world);
	}
	else
		cell = world->grid[x][y];

	return cell;
}

struct Cell *rmCell(struct Cell *cell, struct World *world)
{
	list_del(&cell->lh);
	world->grid[cell->x][cell->y] = NULL;

	return cell;
}

static void deleteCell(struct Cell *cell, struct World *world)
{
	list_del(&cell->lh);
	free(cell);
	world->grid[cell->x][cell->y] = NULL;
}


inline void getCellPos(wsize_t *x, wsize_t *y, const struct Cell *cell)
{
	*x = cell->x;
	*y = cell->y;
}

inline struct Cell *getCell(wsize_t x, wsize_t y, const struct World *world)
{
	return world->grid[x][y];
}

inline struct Cell *wit_first(struct World *world)
{
	return list_entry(world->monitoredCells.next, struct Cell, lh);
}

inline bool wit_done(struct Cell *cell, struct World *world)
{
	return &cell->lh != &world->monitoredCells;
}

inline struct Cell *wit_next(struct Cell *cell)
{
	return list_entry(cell->lh.next, struct Cell, lh);
}
