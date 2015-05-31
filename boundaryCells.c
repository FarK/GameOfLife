#include "boundaryCells.h"
#include <mpi.h>

// MPI Datatype needed vars
MPI_Datatype mpi_coords_t;
static const int mpi_coords_nitems = 2;
static int mpi_coords_blocklengths[2] = {1,1};
static MPI_Datatype mpi_coords_types[2] = {MPI_WSIZE_T, MPI_WSIZE_T};
static MPI_Aint mpi_coords_offsets[2] = {
	offsetof(struct Coords, x),
	offsetof(struct Coords, y)
};


struct BoundaryCells *createBoundaryCells(wsize_t size)
{
	struct BoundaryCells *bcells;

	bcells = (struct BoundaryCells *)malloc(sizeof(struct BoundaryCells));
	bcells->toRevive = (struct Coords *)malloc(size*sizeof(struct Coords));
	bcells->toKill = (struct Coords *)malloc(size*sizeof(struct Coords));
	bcells->toKillSize = 0;
	bcells->toReviveSize = 0;

	return bcells;
}

void setupMPIBoundaryCellsDatatypes()
{
	MPI_Type_create_struct(
		mpi_coords_nitems,
		mpi_coords_blocklengths,
		mpi_coords_offsets,
		mpi_coords_types,
		&mpi_coords_t
	);

	MPI_Type_commit(&mpi_coords_t);
}

inline void deleteBoundaryCells(struct BoundaryCells *bcells)
{
	free(bcells->toKill);
	free(bcells->toRevive);
	free(bcells);
}

inline void clearBoundaryCells(struct BoundaryCells *bcells)
{
	bcells->toKillSize = 0;
	bcells->toReviveSize = 0;
}

inline void addToRevive(wsize_t x, wsize_t y, struct BoundaryCells *bcells)
{
	bcells->toRevive[bcells->toReviveSize].x = x;
	bcells->toRevive[bcells->toReviveSize].y = y;
	++(bcells->toReviveSize);
}

inline void addToKill(wsize_t x, wsize_t y, struct BoundaryCells *bcells)
{
	bcells->toKill[bcells->toKillSize].x = x;
	bcells->toKill[bcells->toKillSize].y = y;
	++(bcells->toKillSize);
}

inline wsize_t getBoundaryTotalSize(const struct BoundaryCells *bcells)
{
	return bcells->toReviveSize + bcells->toKillSize;
}
