#include "node.h"
#include "gol.h"
#include <omp.h>
#include <stdlib.h>
#include <mpi.h>

#include "utils.h"

struct MPINode {
	struct World *world;
	int numProc;
	int ownId;
	int neighborIds[2];

	struct Boundary *RXboundary;
	struct Boundary *TXboundary;
};

static void freeNode(struct MPINode *node);
static bool receiveBound(enum WorldBound bound, enum BoundaryType btype,
	struct MPINode *node);
static bool sendBound(enum WorldBound bound, enum BoundaryType btype,
	struct MPINode *node);
static void receiveBounds(struct MPINode *node);
static void sendBounds(struct MPINode *node);

struct MPINode *createNode(wsize_t ws_x, wsize_t ws_y, int numThreads)
{
	struct MPINode *node;

	node = (struct MPINode *)malloc(sizeof(struct MPINode));

	MPI_Init(NULL, NULL);

	if (numThreads == 0) numThreads = omp_get_max_threads();
	golInit(numThreads);

	MPI_Comm_size(MPI_COMM_WORLD, &node->numProc);

	ws_x = (ws_x/(double)node->numProc) + 0.5;

	if (node->numProc > 1) {
		MPI_Comm_rank(MPI_COMM_WORLD, &node->ownId);
		node->neighborIds[WB_TOP] =
			node->ownId? node->ownId - 1 : node->numProc - 1;
		node->neighborIds[WB_BOTTOM] =(node->ownId + 1) % node->numProc;

		node->world = createWorld(ws_x, ws_y, true);

		getBoundaries(&node->TXboundary, &node->RXboundary,node->world);
	} else
		node->world = createWorld(ws_x, ws_y, false);

	return node;
}

inline static void freeNode(struct MPINode *node)
{
	destroyWorld(node->world);
	golEnd();
	free(node);
}

void deleteNode(struct MPINode *node)
{
	freeNode(node);
	MPI_Finalize();
}

inline void nodeAbort(struct MPINode *node)
{
	if (node) freeNode(node);
	MPI_Abort(MPI_COMM_WORLD, -1);
}

inline static bool receiveBound(enum WorldBound bound, enum BoundaryType btype,
	struct MPINode *node)
{
	int err;
	MPI_Status status;

	err = MPI_Recv(
		node->RXboundary->boundaries[bound][btype],
		boundaryMaxSize,
		MPI_WSIZE_T,
		node->neighborIds[bound],
		node->ownId,
		MPI_COMM_WORLD,
		&status
	);

	MPI_Get_count(&status, MPI_WSIZE_T,
		(int *)&(node->RXboundary->boundariesSizes[bound][btype]));

	return err == MPI_SUCCESS;
}

inline static bool sendBound(enum WorldBound bound, enum BoundaryType btype,
	struct MPINode *node)
{
	int err;
	MPI_Status status;

	err = MPI_Send(
		node->TXboundary->boundaries[bound][btype],
		node->TXboundary->boundariesSizes[bound][btype],
		MPI_WSIZE_T,
		node->neighborIds[bound],
		node->neighborIds[bound],
		MPI_COMM_WORLD
	);

	return err;
}

inline static void receiveBounds(struct MPINode *node)
{
	// Receive boundaries
	receiveBound(WB_TOP,    TO_REVIVE, node);
	setBoundary(WB_TOP, TO_REVIVE, node->world);

	receiveBound(WB_BOTTOM, TO_REVIVE, node);
	setBoundary(WB_TOP, TO_REVIVE, node->world);

	receiveBound(WB_TOP,    TO_KILL,   node);
	setBoundary(WB_TOP, TO_REVIVE, node->world);

	receiveBound(WB_BOTTOM, TO_KILL,   node);
	setBoundary(WB_TOP, TO_REVIVE, node->world);
}

inline static void sendBounds(struct MPINode *node)
{
	// Send boundaries
	sendBound(WB_TOP,    TO_REVIVE, node);
	sendBound(WB_TOP,    TO_REVIVE, node);
	sendBound(WB_BOTTOM, TO_KILL,   node);
	sendBound(WB_BOTTOM, TO_KILL,   node);
}

inline void iterate(struct MPINode *node)
{
	static int it = WB_TOP;

	if (node->numProc > 1) {
		sendBounds(node);
		receiveBounds(node);
		clearBoundaries(node->world);
	}

	iteration(node->world, &rule_B3S23);
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

inline int getNodeId(struct MPINode *node)
{
	return node->ownId;
}
