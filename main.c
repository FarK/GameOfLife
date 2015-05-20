#include <stdio.h>
#include "world.h"
#include "gol.h"

void printCells(struct World *world);
void printWorld(struct World *world);

int main(int nargs, char *argv[])
{
	struct World *world = NULL;

	golInit(2);

	world = createWorld(15, 15, WB_RIGHT);
	printf("Mundo creado = %p\n", world);

	printf("Game of life:\n");
	reviveCell(0, 0, world);
	reviveCell(0, 1, world);
	reviveCell(0, 2, world);

	printWorld(world);

	printf("Iteration 1\n");
	iteration(world, &rule_B3S23);
	printWorld(world);

	printf("Iteration 2\n");
	iteration(world, &rule_B3S23);
	printWorld(world);

	destroyWorld(world);
	golEnd();

	return 0;
}

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

	getSize(&x,&y, world);

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
				numRef = dgetCellRefs(i, j, world);
				printf("%s%d ", alive?"A":"d", numRef);
			}
			else
				printf(" n ");
		}
		printf("\n");
	}
}
