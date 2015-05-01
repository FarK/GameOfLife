#include "world.h"
#include <stdlib.h>
#include <string.h>

// Macro for disable unused warnings
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))

struct World {
	wsize_t x;
	wsize_t y;

	bool **cells;
};


struct World *createWorld(wsize_t x, wsize_t y)
{
	struct World *world;
	bool *cells;
	wsize_t i;

	// Allocate memory
	world = (struct World *) malloc(sizeof(struct World));
	world->cells = (bool **)malloc(x * sizeof(bool *));
	cells = (bool *)malloc(x * y * sizeof(bool));

	// Initialize pointers
	for (i = 0; i < x; ++i)
		world->cells[i] = &cells[i];

	// Initialize struct
	world->x = x;
	world->y = y;

	return world;
}

inline void destroyWorld(struct World *world)
{
	free(world->cells);
	free(world);
}

inline void clearWorld(struct World *world)
{
	memset(world->cells[0], 0, world->x * world->y);
}

inline bool getCell(wsize_t x, wsize_t y, struct World *world)
{
	return world->cells[x][y];
}

inline void setCell(wsize_t x, wsize_t y, bool state, struct World *world)
{
	world->cells[x][y] = state;
}
