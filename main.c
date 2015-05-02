#include <stdio.h>
#include "world.h"

int main(int nargs, char *argv[])
{
	struct World *world = NULL;
	struct Cell *cell = NULL;

	world = createWorld(50, 50);
	printf("Mundo creado = %p\n", world);
	clearWorld(world);
	printf("Mundo inicializado\n");

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


	destroyWorld(world);

	return 0;
}
