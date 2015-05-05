#ifndef WORLD_H_
#define WORLD_H_

#include <stdint.h>
#include <stdbool.h>

typedef long int wsize_t;
struct World;
struct Cell;

struct World *createWorld(wsize_t x, wsize_t y);
void destroyWorld(struct World *world);
void clearWorld(struct World *world);

void getSize(wsize_t *x, wsize_t *y, const struct World *world);

struct Cell *reviveCell(wsize_t x, wsize_t y, struct World *world);
struct Cell *killCell(struct Cell *cell, struct World *world);
struct Cell *getCell(wsize_t x, wsize_t y, const struct World *world);
void getCellPos(wsize_t *x, wsize_t *y, const struct Cell *cell);
bool isCellAlive(const struct Cell *cell);
bool isCellAlive_coord(wsize_t x, wsize_t y,
	const struct World *world);

struct Cell *wit_first(struct World *world);
struct Cell *wit_first_safe(struct World *world, struct Cell **tmp);
bool wit_done(struct Cell *cell, struct World *world);
struct Cell *wit_next(struct Cell *cell);
struct Cell *wit_next_safe(struct Cell **tmp);

#endif
