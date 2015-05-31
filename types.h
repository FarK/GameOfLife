#ifndef TYPES_H_
#define TYPES_H_

#include <mpi.h>

#define MPI_WSIZE_T MPI_LONG_LONG_INT
typedef long int wsize_t;
extern MPI_Datatype mpi_coords_t;

#define TOP     0
#define LEFT    0
#define BOTTOM  1
#define RIGHT   1

enum WorldBound {
	WB_TOP = 0,
	WB_BOTTOM = 1,
	WB_NONE
};

static const char *WorldBound2Str[] = {
	"WB_TOP",
	"WB_BOTTOM",
	"WB_LEFT",
	"WB_RIGHT"
};

static const char *WorldLimits2Str[] = {
	"WL_NONE",
	"WL_TOP_BOTTOM",
	"WL_LEFT_RIGHT"
};

#endif
