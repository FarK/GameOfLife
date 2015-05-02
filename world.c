#include "world.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>

// Macro for disable unused warnings
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))

struct World {
	wsize_t x;
	wsize_t y;

	bool **grid;
	struct list_head cells;
};

struct AliveCell {
	struct list_head lh;
	wsize_t x;
	wsize_t y;
};


struct World *createWorld(wsize_t x, wsize_t y)
{
	struct World *world;
	bool *grid;
	wsize_t i;

	// Allocate memory
	world = (struct World *) malloc(sizeof(struct World));
	world->grid = (bool **)malloc(x * sizeof(bool *));
	grid = (bool *)malloc(x * y * sizeof(bool));

	// Initialize pointers
	for (i = 0; i < x; ++i)
		world->grid[i] = &grid[i];

	// Initialize struct
	world->x = x;
	world->y = y;
	INIT_LIST_HEAD(&world->cells);

	return world;
}

inline void destroyWorld(struct World *world)
{
	struct AliveCell *counter, *tmp;

	list_for_each_entry_safe(counter, tmp, &world->cells, lh)
	free(world->grid);
	free(world);
}

inline void clearWorld(struct World *world)
{
	memset(world->grid[0], 0, world->x * world->y);
}

inline bool getGridCell(wsize_t x, wsize_t y, const struct World *world)
{
	return world->grid[x][y];
}

inline void setGridCell(wsize_t x, wsize_t y, bool state, struct World *world)
{
	world->grid[x][y] = state;
}

void addAliveCell(struct AliveCell *cell, struct World *world)
{
	list_add(&cell->lh, &world->cells);
}

void addNewAliveCell(wsize_t x, wsize_t y, struct World *world)
{
	struct AliveCell *cell;

	if (world->grid[x][y] != true) {
		world->grid[x][y] = true;
		cell = (struct AliveCell *)malloc(sizeof(struct AliveCell));
		cell->x = x;
		cell->y = y;
		addAliveCell(cell, world);
	}
}

void rmAliveCell(struct AliveCell *cell, struct World *world)
{
	list_del(&cell->lh);
	world->grid[cell->x][cell->y] = false;
}

inline void getSize(wsize_t *x, wsize_t *y, const struct World *world)
{
	*x = world->x;
	*y = world->y;

	return;
}
