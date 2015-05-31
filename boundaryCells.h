#ifndef BOUNDARY_CELLS_H_
#define BOUNDARY_CELLS_H_

#include <stdlib.h>
#include <mpi.h>
#include "types.h"

struct Coords {
	wsize_t x;
	wsize_t y;
};

struct BoundaryCells {
	struct Coords *toRevive;
	struct Coords *toKill;
	wsize_t toReviveSize;
	wsize_t toKillSize;
};

extern MPI_Datatype mpi_coords_t;
extern MPI_Datatype mpi_bsizes_t;
extern const unsigned int sizeof_BoundarySize;


struct BoundaryCells *createBoundaryCells(wsize_t size);
void setupMPIBoundaryCellsDatatypes();
inline void deleteBoundaryCells(struct BoundaryCells *bcells);
inline void clearBoundaryCells(struct BoundaryCells *bcells);
inline void addToRevive(wsize_t x, wsize_t y, struct BoundaryCells *bcells);
inline void addToKill(wsize_t x, wsize_t y, struct BoundaryCells *bcells);
void getToReviveCell(wsize_t idx, wsize_t *x, wsize_t *y,
	const struct BoundaryCells *bcells);
void getToKillCell(wsize_t idx, wsize_t *x, wsize_t *y,
	const struct BoundaryCells *bcells);
wsize_t getBoundaryTotalSize(const struct BoundaryCells *bcells);

#endif
