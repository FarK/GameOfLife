#include "node.h"
#include "gol.h"
#include "boundaryCells.h"
#include <stdlib.h>
#include <mpi.h>

#include "utils.h"

struct MPINode {
	struct World *world;
	struct BoundaryCells *TXbCells[2];
	struct BoundaryCells *RXbCells[2];
	wsize_t boundaryMaxSize;
	int numProc;
	int ownId;
	int neighborIds[2];
};

struct MPINode *createNode(wsize_t ws_x, wsize_t ws_y)
{
	struct MPINode *node;

	node = (struct MPINode *)malloc(sizeof(struct MPINode));

	MPI_Init(NULL, NULL);
	setupMPIBoundaryCellsDatatypes();
	golInit(1);

	MPI_Comm_size(MPI_COMM_WORLD, &node->numProc);

	if (node->numProc > 1) {
		MPI_Comm_rank(MPI_COMM_WORLD, &node->ownId);
		node->neighborIds[WB_TOP] =
			node->ownId? node->ownId - 1 : node->numProc - 1;
		node->neighborIds[WB_BOTTOM] =(node->ownId + 1) % node->numProc;

		node->boundaryMaxSize = ws_y;

		node->TXbCells[WB_TOP] =
			createBoundaryCells(node->boundaryMaxSize);
		node->TXbCells[WB_BOTTOM] =
			createBoundaryCells(node->boundaryMaxSize);
		node->RXbCells[WB_TOP] =
			createBoundaryCells(node->boundaryMaxSize);
		node->RXbCells[WB_BOTTOM] =
			createBoundaryCells(node->boundaryMaxSize);

		node->world = createWorld(ws_x, ws_y, true);
	} else
		/* node->world = createWorld(ws_x, ws_y, WL_NONE); */
		node->world = createWorld(ws_x, ws_y, false);

	return node;
}

void deleteNode(struct MPINode *node)
{
	destroyWorld(node->world);
	if (node->numProc > 1) {
		printf("DELETING BOUNDARIES\n");
		deleteBoundaryCells(node->TXbCells[WB_TOP]);
		deleteBoundaryCells(node->TXbCells[WB_BOTTOM]);
		deleteBoundaryCells(node->RXbCells[WB_TOP]);
		deleteBoundaryCells(node->RXbCells[WB_BOTTOM]);
	}
	golEnd();
	free(node);
}

static inline bool receiveBounds(struct MPINode *node)
{
	int err;
	MPI_Status status;

	// Receive toRevive first boundary
	err = MPI_Recv(
		node->RXbCells[WB_TOP]->toRevive,
		node->boundaryMaxSize,
		mpi_coords_t,
		node->neighborIds[WB_TOP],
		node->ownId,
		MPI_COMM_WORLD,
		&status
	);
	MPI_Get_count(&status, mpi_coords_t,
		(int *)&(node->RXbCells[WB_TOP]->toReviveSize));
	if (node->RXbCells[WB_TOP]->toReviveSize) {
		printf("\tNode %d <- %d: receive toRevive WB_TOP: Size = %ld\n", node->ownId, node->neighborIds[WB_TOP], node->RXbCells[WB_TOP]->toReviveSize);
		printf("\tNode %d <- %d: (%ld, %ld)\n", node->ownId, node->neighborIds[WB_TOP], node->RXbCells[WB_TOP]->toRevive[0].x, node->RXbCells[WB_TOP]->toRevive[0].y);
	}

	// Receive toKill first boundary
	err = MPI_Recv(
		node->RXbCells[WB_TOP]->toKill,
		node->boundaryMaxSize,
		mpi_coords_t,
		node->neighborIds[WB_TOP],
		node->ownId,
		MPI_COMM_WORLD,
		&status
	);
	MPI_Get_count(&status, mpi_coords_t,
		(int *)&(node->RXbCells[WB_TOP]->toKillSize));
	if (node->RXbCells[WB_TOP]->toKillSize)
		printf("\tNode %d <- %d: receive toKill WB_TOP: Size = %ld\n", node->ownId, node->neighborIds[WB_TOP], node->RXbCells[WB_TOP]->toKillSize);

	// Receive toRevive second boundary
	err = MPI_Recv(
		node->RXbCells[WB_BOTTOM]->toRevive,
		node->boundaryMaxSize,
		mpi_coords_t,
		node->neighborIds[WB_BOTTOM],
		node->ownId,
		MPI_COMM_WORLD,
		&status
	);
	MPI_Get_count(&status, mpi_coords_t,
		(int *)&(node->RXbCells[WB_BOTTOM]->toReviveSize));
	if (node->RXbCells[WB_BOTTOM]->toReviveSize)
		printf("\tNode %d <- %d: receive toRevive WB_BOTTOM: Size = %ld\n", node->ownId, node->neighborIds[WB_BOTTOM], node->RXbCells[WB_BOTTOM]->toReviveSize);

	// Receive toKill second boundary
	err = MPI_Recv(
		node->RXbCells[WB_BOTTOM]->toKill,
		node->boundaryMaxSize,
		mpi_coords_t,
		node->neighborIds[WB_BOTTOM],
		node->ownId,
		MPI_COMM_WORLD,
		&status
	);
	MPI_Get_count(&status, mpi_coords_t,
		(int *)&(node->RXbCells[WB_BOTTOM]->toKillSize));
	if (node->RXbCells[WB_BOTTOM]->toKillSize)
		printf("\tNode %d <- %d: receive toKill WB_BOTTOM: Size = %ld\n", node->ownId, node->neighborIds[WB_BOTTOM], node->RXbCells[WB_TOP]->toKillSize);

	setBoundaryCells(node->RXbCells[WB_TOP], node->world);
	setBoundaryCells(node->RXbCells[WB_BOTTOM], node->world);

	return err == MPI_SUCCESS;
}

static inline bool sendBounds(struct MPINode *node)
{
	int err;

	// Send toRevive first boundary
	err = MPI_Send(
		node->TXbCells[WB_TOP]->toRevive,
		node->TXbCells[WB_TOP]->toReviveSize,
		mpi_coords_t,
		node->neighborIds[WB_TOP],
		node->neighborIds[WB_TOP],
		MPI_COMM_WORLD
	);
	if (node->TXbCells[WB_TOP]->toReviveSize)
		printf("\tNode %d -> %d: send toRevive WB_TOP: Size = %ld\n", node->ownId, node->neighborIds[WB_TOP], node->TXbCells[WB_TOP]->toReviveSize);

	// Send toKill fisrt boundary
	err = MPI_Send(
		node->TXbCells[WB_TOP]->toKill,
		node->TXbCells[WB_TOP]->toKillSize,
		mpi_coords_t,
		node->neighborIds[WB_TOP],
		node->neighborIds[WB_TOP],
		MPI_COMM_WORLD
	);
	if (node->TXbCells[WB_TOP]->toKillSize)
		printf("\tNode %d -> %d: send toKill WB_TOP: Size = %ld\n", node->ownId, node->neighborIds[WB_TOP], node->TXbCells[WB_TOP]->toKillSize);

	// Send toRevive second boundary
	err = MPI_Send(
		node->TXbCells[WB_BOTTOM]->toRevive,
		node->TXbCells[WB_BOTTOM]->toReviveSize,
		mpi_coords_t,
		node->neighborIds[WB_BOTTOM],
		node->neighborIds[WB_BOTTOM],
		MPI_COMM_WORLD
	);
	if (node->TXbCells[WB_BOTTOM]->toReviveSize)
		printf("\tNode %d -> %d: send toRevive WB_BOTTOM: Size = %ld\n", node->ownId, node->neighborIds[WB_BOTTOM], node->TXbCells[WB_BOTTOM]->toReviveSize);

	// Send toKill second boundary
	err = MPI_Send(
		node->TXbCells[WB_BOTTOM]->toKill,
		node->TXbCells[WB_BOTTOM]->toKillSize,
		mpi_coords_t,
		node->neighborIds[WB_BOTTOM],
		node->neighborIds[WB_BOTTOM],
		MPI_COMM_WORLD
	);
	if (node->TXbCells[WB_BOTTOM]->toKillSize)
		printf("\tNode %d -> %d: send toKill WB_BOTTOM: Size = %ld\n", node->ownId, node->neighborIds[WB_BOTTOM], node->TXbCells[WB_BOTTOM]->toKillSize);


	return err == MPI_SUCCESS;
}

inline void iterate(struct MPINode *node)
{
	static int it = WB_TOP;

	if (node->numProc > 1) {
		sendBounds(node);

		receiveBounds(node);

		clearBoundaryCells(node->TXbCells[WB_TOP]);
		clearBoundaryCells(node->TXbCells[WB_BOTTOM]);
	}

	iteration(node->world, &rule_B3S23, node->TXbCells);
	printf("ITERATION %d - NODE %d\n", it++, node->ownId);
	printWorld(node->world);
}

inline void node_reviveCell(wsize_t x, wsize_t y, struct MPINode *node)
{
	gol_reviveCell(x, y, node->world);
}

inline void node_killCell(wsize_t x, wsize_t y, struct MPINode *node)
{
	gol_killCell(x, y, node->world);
}

void printNode(struct MPINode *node)
{
	printf("NODE %d\n", node->ownId);
	printWorld(node->world);
}
