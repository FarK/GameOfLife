#ifndef MALLOC_H_
#define MALLOC_H_

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

inline static void *mallocC(size_t size)
{
	void *ptr = malloc(size);

	if (!ptr) {
		fprintf(stderr, "Can't reserve memory\n");
		MPI_Abort(MPI_COMM_WORLD, -1);
	}

	return ptr;
}

#endif
