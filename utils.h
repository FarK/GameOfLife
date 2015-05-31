#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdarg.h>
#include <mpi.h>
#include "world.h"

void printCells(struct World *world)
{
	int i = 0;
	wsize_t x, y;
	struct Cell *cell, *tmp = NULL;

	for (cell = wit_first_safe(world, &tmp);
	     wit_done(cell, world);
	     cell = wit_next_safe(&tmp))
	{
		getCellPos(&x, &y, cell);
		printf("%2d - (%2lu, %2lu) = %02X\tref=%d\n",
			i++, x, y, isCellAlive(cell), dgetCellRefs(x,y,world));
	}
}

void printWorld(struct World *world)
{
	int i,j;
	wsize_t x,y;
	struct Cell *cell;
	bool alive;
	char numRef;

	getRealSize(&x,&y, world);

	printf("    ");
	for (j = 0; j < y; j++)
		printf("%2d ", j);
	printf("\n");

	for (i = 0; i < x; i++) {
		printf("%2d  ", i);
		for (j = 0; j < y; j++) {
			cell = getCell(i, j, world);
			if (cell != NULL) {
				alive = isCellAlive(cell);
				/*
				 * numRef = dgetCellRefs(i, j, world);
				 * printf("%s%d ", alive?"A":"d", numRef);
				 */
				printf(" %c ", alive?'#':'o');
			}
			else
				// printf(" n ");
				printf(" · ");
		}
		printf("\n");
	}
}

// TODO: no funca ¿paralelismo?
void mpipf(const char *fmt, ...)
{
	static int nodes[5] = {0, 0, 0, 0, 0};
	int rank;
	va_list ap;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	printf("%d.%03d:\t", rank, nodes[rank]++);
	va_start(ap, fmt);
	printf(fmt, ap);
	va_end(ap);
	printf("\n");
}

#endif
