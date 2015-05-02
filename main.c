#include <stdio.h>
#include "world.h"

int main(int nargs, char *argv[])
{
	struct World *world = NULL;

	world = createWorld(50, 50);
	printf("Mundo creado = %p\n", world);
	clearWorld(world);
	printf("Mundo inicializado\n");

	printf("Añadiendo célula\n");
	addNewAliveCell(4, 4, world);
	printf("Cell[4,4] = %d\n", getGridCell(4, 4, world));

	destroyWorld(world);
}
