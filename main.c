#include <stdio.h>
#include "world.h"

void printCells(struct World *world);

int main(int nargs, char *argv[])
{
	struct World *world = NULL;
	struct Cell *cell = NULL;

	world = createWorld(50, 50);
	printf("Mundo creado = %p\n", world);

	printf("Añadiendo célula\n");
	cell = addNewCell(4, 4, world);
	printf("Cell[4,4] = %s\n",
		getCell(4, 4, world) != NULL? "alive" : "dead");
	printf("Cell[1,1] = %s\n",
		getCell(1, 1, world) != NULL? "alive" : "dead");

	cell = rmCell(cell, world);
	printf("Cell[4,4] = %s\n",
		getCell(4, 4, world) != NULL? "alive" : "dead");

	setCellPos(1, 1, cell);
	addCell(cell, world);
	printf("Cell[1,1] = %s\n",
		getCell(1, 1, world) != NULL? "alive" : "dead");


	addNewCell(1, 2, world);
	addNewCell(1, 3, world);
	addNewCell(1, 3, world);
	addNewCell(2, 1, world);
	addNewCell(2, 2, world);
	addNewCell(2, 3, world);
	addNewCell(4, 3, world);
	addNewCell(9, 9, world);

	printCells(world);

	clearWorld(world);
	printf("Mundo borrado: \n");

	destroyWorld(world);

	return 0;
}

void printCells(struct World *world)
{
	int i = 0;
	wsize_t x, y;
	struct Cell *cell;

	for (cell = wit_first(world);
	     wit_done(cell, world);
	     cell = wit_next(cell))
	{
		getCellPos(&x, &y, cell);
		printf("%d - (%lu, %lu)\n", i++, x, y);
	}
}
