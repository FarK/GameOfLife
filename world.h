#ifndef WORLD_H_
#define WORLD_H_

#include <stdint.h>
#include <stdbool.h>

typedef unsigned long wsize_t;
struct World;

struct World *createWorld(wsize_t x, wsize_t y);
void destroyWorld(struct World *world);
void clearWorld(struct World *world);
bool getCell(wsize_t x, wsize_t y, struct World *world);
void setCell(wsize_t x, wsize_t y, bool state, struct World *world);

#endif
