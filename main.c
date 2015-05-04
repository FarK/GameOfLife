#include <stdio.h>
#include "world.h"

void printCells(struct World *world);

int main(int nargs, char *argv[])
{
	struct World *world = NULL;

	world = createWorld(50, 50);
	printf("Mundo creado = %p\n", world);

	printf("Añadiendo células\n");

	addNewAliveCell(1, 1, world);
	addNewAliveCell(-1, 1, world);
	printf("Dos células:\n");
	printCells(world);

	rmCell(getCell(1, 1, world), world);
	printf("\nUna célula:\n");
	printCells(world);
	rmCell(getCell(-1, 1, world), world);
	printf("\nNinguna célula:\n");
	printCells(world);

	addNewAliveCell(1, 2, world);
	addNewAliveCell(1, 3, world);
	addNewAliveCell(1, 3, world);
	addNewAliveCell(2, 1, world);

	printf("\nMuchas células:\n");
	printCells(world);

	clearWorld(world);
	printf("Mundo borrado: \n");
	printCells(world);

	addNewAliveCell(2, 2, world);
	addNewAliveCell(2, 3, world);
	addNewAliveCell(4, 3, world);
	addNewAliveCell(9, 9, world);

	destroyWorld(world);

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
		printf("%d - (%lu, %lu) = %02X\n",
			i++, x, y, getCellState(cell));
	}
}
