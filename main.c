#include <stdio.h>
#include "world.h"

int main(int nargs, char *argv[])
{
	struct World *world = NULL;

	world = createWorld(50, 50);
	printf("Mundo creado = %p\n", world);
	clearWorld(world);
	printf("Mundo inicializado\n");

	setCell(4, 4, true, world);
	printf("Mundo setCell\n");
	printf("Cell[4,4] = %d\n", getCell(4, 4, world));

	destroyWorld(world);
}
