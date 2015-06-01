#include "world.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>

// Macro for disable unused warnings
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))

#define NB_TOP (1<<0)
#define NB_MID (1<<1)
#define NB_BOT (1<<2)
#define NB_ALL (NB_TOP | NB_MID | NB_BOT)

struct World {
	wsize_t x;
	wsize_t y;

	unsigned char limits;
	struct Boundary *TXBoundary;
	struct Boundary *RXBoundary;

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

unsigned int boundaryMaxSize;

// Auxiliary functions
static struct Cell *newCell(wsize_t x, wsize_t y, unsigned char num_ref,
	bool alive);
static void addCell(struct Cell *cell, struct World *world);
static void deleteCell(struct Cell *cell, struct World *world);
static void setNeighbor(wsize_t x, wsize_t y, unsigned bound,
	void (*setRef)(wsize_t, wsize_t, struct World *),
	struct World *world);
static void incRef(wsize_t x, wsize_t y, struct World *world);
static void decRef(wsize_t x, wsize_t y, struct World *world);
static void toroidalCoords(wsize_t *x, wsize_t *y, const struct World *world);
static struct Boundary *createBoundary();
static void freeBoundary(struct Boundary *boundary);
static void addToBoundary(wsize_t y, enum WorldBound bound,
	enum BoundaryType btype, struct Boundary *boundary);


struct World *createWorld(wsize_t x, wsize_t y, unsigned char limits)
{
	struct World *world;
	struct Cell **grid;
	wsize_t i, j;

	// Allocate memory
	world = (struct World *) malloc(sizeof(struct World));
	world->grid = (struct Cell ***)malloc(x * sizeof(struct Cell *));
	grid = (struct Cell **)malloc(x * y * sizeof(struct Cell *));

	if (limits) {
		boundaryMaxSize = y * sizeof(wsize_t);
		world->RXBoundary = createBoundary(boundaryMaxSize);
		world->TXBoundary = createBoundary(boundaryMaxSize);
	}

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
	INIT_LIST_HEAD(&world->monitoredCells);
	world->numMonCells = 0;

	return world;
}

static struct Boundary *createBoundary()
{
	struct Boundary *boundary;

	boundary = (struct Boundary *)malloc(sizeof(struct Boundary));

	boundary->boundaries[WB_TOP][TO_REVIVE] =
		(wsize_t *)malloc(boundaryMaxSize);
	boundary->boundaries[WB_TOP][TO_KILL] =
		(wsize_t *)malloc(boundaryMaxSize);
	boundary->boundaries[WB_BOTTOM][TO_REVIVE] =
		(wsize_t *)malloc(boundaryMaxSize);
	boundary->boundaries[WB_BOTTOM][TO_KILL] =
		(wsize_t *)malloc(boundaryMaxSize);


	boundary->boundariesSizes[WB_TOP][TO_REVIVE] = 0;
	boundary->boundariesSizes[WB_TOP][TO_KILL] = 0;
	boundary->boundariesSizes[WB_BOTTOM][TO_REVIVE] = 0;
	boundary->boundariesSizes[WB_BOTTOM][TO_KILL] = 0;

	return boundary;
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

	if (world->limits) {
		freeBoundary(world->TXBoundary);
		freeBoundary(world->RXBoundary);
	}
	free(world);
}

inline static void freeBoundary(struct Boundary *boundary)
{
	free(boundary->boundaries[WB_TOP][TO_REVIVE]);
	free(boundary->boundaries[WB_TOP][TO_KILL]);
	free(boundary->boundaries[WB_BOTTOM][TO_REVIVE]);
	free(boundary->boundaries[WB_BOTTOM][TO_KILL]);

	free(boundary);
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
	clearBoundaries(world);
}

inline void clearBoundaries(struct World *world)
{
	world->TXBoundary->boundariesSizes[WB_TOP   ][TO_REVIVE] = 0;
	world->TXBoundary->boundariesSizes[WB_TOP   ][TO_KILL  ] = 0;
	world->TXBoundary->boundariesSizes[WB_BOTTOM][TO_REVIVE] = 0;
	world->TXBoundary->boundariesSizes[WB_BOTTOM][TO_KILL  ] = 0;

	world->RXBoundary->boundariesSizes[WB_TOP   ][TO_REVIVE] = 0;
	world->RXBoundary->boundariesSizes[WB_TOP   ][TO_KILL  ] = 0;
	world->RXBoundary->boundariesSizes[WB_BOTTOM][TO_REVIVE] = 0;
	world->RXBoundary->boundariesSizes[WB_BOTTOM][TO_KILL  ] = 0;
}

inline void getSize(wsize_t *x, wsize_t *y, const struct World *world)
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

	toroidalCoords(&x, &y, world);

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

	toroidalCoords(&x, &y, world);

	cell = world->grid[x][y];
	if (cell == NULL) return;

	--(cell->num_ref);
	if (!cell->alive && cell->num_ref <= 0)
		deleteCell(cell, world);
}

static void setNeighbor(wsize_t x, wsize_t y, unsigned bound,
	void (*setRef)(wsize_t, wsize_t, struct World *),
	struct World *world)
{
	if (bound & NB_TOP) {
		setRef(x+1, y, world);
		setRef(x+1, y-1, world);
		setRef(x+1, y+1, world);
	}
	if (bound & NB_MID) {
		setRef(x, y-1, world);
		setRef(x, y+1, world);
	}
	if (bound & NB_BOT) {
		setRef(x-1, y, world);
		setRef(x-1, y-1, world);
		setRef(x-1, y+1, world);
	}
}

void reviveCell(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;
	unsigned bound = NB_ALL;

	if (world->limits) {
		if (x == 0) {
			addToBoundary(y, WB_TOP, TO_REVIVE, world->TXBoundary);
			bound = NB_TOP | NB_MID;
		} else if (x == world->x-1) {
			addToBoundary(y, WB_BOTTOM, TO_REVIVE, world->TXBoundary);
			bound = NB_BOT | NB_MID;
		}
	}

	cell = world->grid[x][y];

	if (cell == NULL) {
		cell = newCell(x, y, 0, true);
		addCell(cell, world);
		setNeighbor(x, y, bound, incRef, world);
	}
	else if (!cell->alive) {
		setNeighbor(x, y, bound, incRef, world);
		cell->alive = true;
	}
}

void reviveCells(struct list_head *list, struct World *world)
{
	struct CellListNode *cln;

	list_for_each_entry(cln, list, lh)
		reviveCell(cln->cell->x, cln->cell->y, world);
}

void killCell(wsize_t x, wsize_t y, struct World *world)
{
	struct Cell *cell;
	unsigned bound = NB_ALL;

	if (world->limits) {
		if (x == 0) {
			addToBoundary(y, WB_TOP, TO_KILL, world->TXBoundary);
			bound = NB_TOP | NB_MID;
		} else if (x == world->x-1) {
			addToBoundary(y, WB_BOTTOM, TO_KILL, world->TXBoundary);
			bound = NB_BOT | NB_MID;
		}
	}

	cell = world->grid[x][y];

	setNeighbor(x, y, bound, decRef, world);
	if (cell != NULL && cell->alive) {
		cell->alive = false;
	}
}

void killCells(struct list_head *list, struct World *world)
{
	struct CellListNode *cln;

	list_for_each_entry(cln, list, lh)
		killCell(cln->cell->x, cln->cell->y, world);
}

static void deleteCell(struct Cell *cell, struct World *world)
{
	list_del(&cell->lh);
	world->grid[cell->x][cell->y] = NULL;
	free(cell);
	--(world->numMonCells);
}

inline static void toroidalCoords(wsize_t *x, wsize_t *y,
	const struct World *world)
{
	if      (*x < 0)         *x = world->x + *x;
	else if (*x >= world->x) *x = *x - world->x;

	if      (*y < 0)         *y = world->y + *y;
	else if (*y >= world->y) *y = *y - world->y;
}

inline static void addToBoundary(wsize_t y, enum WorldBound bound,
	enum BoundaryType btype, struct Boundary *boundary)
{
	wsize_t indx = boundary->boundariesSizes[bound][btype]++;
	boundary->boundaries[bound][btype][indx] = y;
}

void setBoundary(enum WorldBound bound, enum BoundaryType btype,
	struct World *world)
{
	int i;
	wsize_t x_coord;
	wsize_t y_coord;
	wsize_t bsize;
	void (*setRef)(wsize_t, wsize_t, struct World *);
	unsigned neighborBounds;

	bsize = world->RXBoundary->boundariesSizes[bound][btype];

	switch (bound) {
		case WB_TOP:
			x_coord = world->x;
			neighborBounds = NB_BOT;
			break;
		case WB_BOTTOM:
			x_coord = -1;
			neighborBounds = NB_TOP;
			break;
		default:
			return;
	};

	switch (btype) {
		case TO_REVIVE:
			setRef = incRef;
			break;
		case TO_KILL:
			setRef = decRef;
			break;
		default:
			return;
	};

	for (i = 0; i < bsize; i++) {
		y_coord = world->RXBoundary->boundaries[bound][btype][i],
		setNeighbor(x_coord, y_coord, neighborBounds, setRef, world);
	}
}

inline void getBoundaries(struct Boundary **tx, struct Boundary **rx,
	const struct World *world)
{
	*tx = world->TXBoundary;
	*rx = world->RXBoundary;
}

char dgetCellRefs(wsize_t x, wsize_t y, const struct World *world)
{
	toroidalCoords(&x, &y, world);
	return world->grid[x][y] == NULL? 0 : world->grid[x][y]->num_ref;
}

inline char getCellRefs(struct Cell *cell)
{
	return cell->num_ref;
}

inline bool isCellAlive(const struct Cell *cell)
{
	return cell->alive;
}

inline bool isCellAlive_coord(wsize_t x, wsize_t y, const struct World *world)
{
	struct Cell *cell;

	cell = world->grid[x][y];

	return cell == NULL? false : cell->alive;
}

enum WorldBound boundsCheck(wsize_t *x, wsize_t *y, const struct World *world)
{
	if (*x == 0) {
		*x = world->x;
		return WB_TOP;
	} else if (*x == world->x-1) {
		*x = -1;
		return WB_BOTTOM;
	}

	return WB_NONE;
}

inline struct Cell *getCell(wsize_t x, wsize_t y, const struct World *world)
{
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

	toroidalCoords(&x, &y, world);

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
