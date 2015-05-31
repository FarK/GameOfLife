#include <stdio.h>
#include "node.h"


int main(int nargs, char *argv[])
{
	struct MPINode *node;

	node = createNode(10, 20, -1);

	node_reviveCell(2,7, node);
	node_reviveCell(2,8, node);
	node_reviveCell(2,9, node);
	node_reviveCell(3,7, node);
	node_reviveCell(4,8, node);
	/*
	 * node_reviveCell(1,2, node);
	 * node_reviveCell(1,3, node);
	 * node_reviveCell(1,4, node);
	 */

	printNode(node);

	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);
	iterate(node);

	deleteNode(node);

	MPI_Finalize();

	return 0;
}
