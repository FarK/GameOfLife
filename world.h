#ifndef WORLD_H_
#define WORLD_H_

#include <stdint.h>
#include <stdbool.h>
#include "types.h"
#include "boundaryCells.h"
#include "list.h"
#include <mpi.h>

struct World;
struct Cell;
struct CellListNode {
	struct list_head lh;
	struct Cell *cell;
};


struct World *createWorld(wsize_t x, wsize_t y, unsigned char limits);
void destroyWorld(struct World *world);
void clearWorld(struct World *world);

void getSize(wsize_t *x, wsize_t *y, const struct World *world);
void getRealSize(wsize_t *x, wsize_t *y, const struct World *world);

void reviveCell(wsize_t x, wsize_t y, struct World *world);
void reviveCells(struct list_head *list, struct World *world);
void killCell(wsize_t x, wsize_t y, struct World *world);
void killCells(struct list_head *list, struct World *world);
struct Cell *getCell(wsize_t x, wsize_t y, const struct World *world);
void getCellPos(wsize_t *x, wsize_t *y, const struct Cell *cell);
char getCellRefs(wsize_t x, wsize_t y, const struct World *world);
char dgetCellRefs(wsize_t x, wsize_t y, const struct World *world);
bool isCellAlive(const struct Cell *cell);
bool isCellAlive_coord(wsize_t x, wsize_t y, const struct World *world);

enum WorldBound boundsCheck(wsize_t *x, wsize_t *y, const struct World *world);
void setBoundaryCells(const struct BoundaryCells *bcells, struct World *world);

void addToList(struct Cell *cell, struct list_head *list);
void addToList_coords(wsize_t x, wsize_t y, bool alive, struct list_head *list,
	struct World *world);
void freeList(struct list_head *list);

struct Cell *wit_first(const struct World *world);
struct Cell *wit_first_safe(const struct World *world, struct Cell **tmp);
struct Cell *wit_first_split(unsigned int *count, unsigned int indx,
	const struct World *world);
bool wit_done(const struct Cell *cell, const struct World *world);
inline bool wit_done_split(unsigned int count, const struct World *world);
struct Cell *wit_next(const struct Cell *cell);
struct Cell *wit_next_safe(struct Cell **tmp);
inline struct Cell *wit_next_split(const struct Cell *cell, unsigned int *count,
	unsigned int splits);

#endif
