#ifndef WORLD_H_
#define WORLD_H_

#include <stdint.h>
#include <stdbool.h>

typedef unsigned long wsize_t;
struct World;
struct AliveCell;

struct World *createWorld(wsize_t x, wsize_t y);
void destroyWorld(struct World *world);
void clearWorld(struct World *world);
bool getGridCell(wsize_t x, wsize_t y, const struct World *world);
void setGridCell(wsize_t x, wsize_t y, bool state, struct World *world);
void addAliveCell(struct AliveCell *cell, struct World *world);
void addNewAliveCell(wsize_t x, wsize_t y, struct World *world);
void rmAliveCell(struct AliveCell *cell, struct World *world);
void getSize(wsize_t *x, wsize_t *y, const struct World *world);

#endif
