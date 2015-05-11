#include <stdio.h>
#include "world.h"
#include "gol.h"

void printCells(struct World *world);

int main(int nargs, char *argv[])
{
	struct World *world = NULL;

	golInit(2);

	world = createWorld(50, 50);
	printf("Mundo creado = %p\n", world);

	printf("Añadiendo células\n");

	reviveCell(1, 1, world);
	reviveCell(-1, 1, world);
	printf("Dos células:\n");
	printCells(world);

	killCell(getCell(1, 1, world), world);
	printf("\nUna célula:\n");
	printCells(world);
	killCell(getCell(-1, 1, world), world);
	printf("\nNinguna célula:\n");
	printCells(world);

	reviveCell(1, 2, world);
	reviveCell(1, 3, world);
	reviveCell(1, 3, world);
	reviveCell(2, 1, world);

	printf("\nMuchas células:\n");
	printCells(world);

	clearWorld(world);
	printf("Mundo borrado: \n");
	printCells(world);

	reviveCell(2, 2, world);
	reviveCell(2, 3, world);
	reviveCell(2, 4, world);

	printf("Game of life:\n");
	printCells(world);

	printf("Iteration 1\n");
	iteration(world, &rule_B3S23);
	printCells(world);

	printf("Iteration 2\n");
	iteration(world, &rule_B3S23);
	printCells(world);

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
		printf("%d - (%lu, %lu) = %02X\n",
			i++, x, y, isCellAlive(cell));
	}
}
