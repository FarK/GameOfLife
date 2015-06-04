#include "node.h"
#include "gol.h"
#include "io.h"
#include "stats.h"
#include "malloc.h"
#include <omp.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_FILENAME 10

struct MPINode {
	struct World *world;
	struct GOL *gol;
	struct Stats *stats;
	const struct Parameters *params;
	int numProc;
	int ownId;
	int neighborIds[2];

	struct Boundary *RXboundary;
	struct Boundary *TXboundary;

	long long unsigned int itCounter;
	char dirName[MAX_FILENAME];
};

static void iterate(struct MPINode *node);
static bool receiveBound(enum WorldBound bound, enum BoundaryType btype,
	struct MPINode *node);
static bool sendBound(enum WorldBound bound, enum BoundaryType btype,
	struct MPINode *node);
static void receiveBounds(struct MPINode *node);
static void sendBounds(struct MPINode *node);
static void treadIOError(struct MPINode *node);

struct MPINode *createNode(const struct Parameters *params, struct Stats *stats)
{
	struct MPINode *node;
	wsize_t x, y;

	node = (struct MPINode *)mallocC(sizeof(struct MPINode));

	MPI_Comm_size(MPI_COMM_WORLD, &node->numProc);
	MPI_Comm_rank(MPI_COMM_WORLD, &node->ownId);

	x = (params->x/(double)node->numProc) + 0.5;
	y = params->y;

	if (node->numProc > 1) {
		node->neighborIds[WB_TOP] =
			node->ownId? node->ownId - 1 : node->numProc - 1;
		node->neighborIds[WB_BOTTOM] =(node->ownId + 1) % node->numProc;

		node->world = createWorld(x, y, true);

		getBoundaries(&node->TXboundary, &node->RXboundary,node->world);
	} else
		node->world = createWorld(params->x, params->y, false);

	node->itCounter = 0;
	node->params = params;
	node->stats = stats;

	snprintf(node->dirName, MAX_FILENAME, "node%d", node->ownId);
	if (!createSubdir(node->dirName)) treadIOError(node);

	node->gol = golInit(params->numThreads, &rule_B3S23, node->world,stats);

	return node;
}

void deleteNode(struct MPINode *node)
{
	destroyWorld(node->world);
	golEnd(node->gol);
	free(node);
}

inline void nodeAbort(struct MPINode *node)
{
	if (node) deleteNode(node);
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
	setBoundary(WB_TOP,     TO_REVIVE, node->world);

	receiveBound(WB_BOTTOM, TO_REVIVE, node);
	setBoundary(WB_BOTTOM,  TO_REVIVE, node->world);

	receiveBound(WB_TOP,    TO_KILL,   node);
	setBoundary(WB_TOP,     TO_KILL, node->world);

	receiveBound(WB_BOTTOM, TO_KILL,   node);
	setBoundary(WB_BOTTOM,  TO_KILL, node->world);
}

inline static void sendBounds(struct MPINode *node)
{
	// Send boundaries
	sendBound(WB_TOP,    TO_REVIVE, node);
	sendBound(WB_BOTTOM, TO_REVIVE, node);
	sendBound(WB_TOP,    TO_KILL,   node);
	sendBound(WB_BOTTOM, TO_KILL,   node);
}

void run(struct MPINode *node)
{
	double pTime, itTime;

	pTime = startMeasurement();

	for (
		node->itCounter = 0;
		node->itCounter <= node->params->iterations;
		++(node->itCounter)
	) {
		itTime = startMeasurement();

		iterate(node);

		endMeasurement(itTime, mpiIteration, node->stats);

		if (node->params->record && !write(node)) treadIOError(node);
	}

	endMeasurement(pTime, total, node->stats);
}

inline static void iterate(struct MPINode *node)
{
	double subItTime, commTime;

	if (node->numProc > 1) {
		commTime = startMeasurement();

		sendBounds(node);
		receiveBounds(node);
		clearBoundaries(node->world);

		endMeasurement(commTime, communication, node->stats);
	}

	subItTime = startMeasurement();

	iteration(node->gol);

	endMeasurement(subItTime, ompIteration, node->stats);
}

inline static void treadIOError(struct MPINode *node)
{
	fprintf(stderr,
		"Can't write output."
		"Please make sure you have enough permmissions\n"
	);
	nodeAbort(node);

	exit(EXIT_FAILURE);
}

inline void node_reviveCell(wsize_t x, wsize_t y, struct MPINode *node)
{
	gol_reviveCell(x, y, node->gol);
}

inline void node_killCell(wsize_t x, wsize_t y, struct MPINode *node)
{
	gol_killCell(x, y, node->gol);
}

bool write(struct MPINode *node)
{
	char filename[MAX_FILENAME];
	bool alive;
	bool ret;
	wsize_t x, y;
	wsize_t i, j;
	char *buffer, *pBuffer;
	size_t buffSize;

	getSize(&x, &y, node->world);
	buffSize = x*(y*2 + 1) + 2;

	buffer = (char *)mallocC(buffSize * sizeof(char));
	if (buffer == NULL) return false;
	pBuffer = buffer;

	// Fill buffer
	for (i = 0; i < x; ++i) {
		for (j = 0; j < y; ++j) {
			struct Cell *cell = getCell(i, j, node->world);
			if (cell == NULL)
				pBuffer += sprintf(pBuffer, ". ");
			else{
				alive = isCellAlive(cell);
				pBuffer += sprintf(pBuffer, "%c ", alive? 'o' : '.');
			}
		}
		pBuffer += sprintf(pBuffer, "\n");
	}
	buffer[buffSize-1] = '\n';
	buffer[buffSize-2] = '\0';

	// Write file
	snprintf(filename, 3+1, "%03Ld", node->itCounter);
	ret = writeBuffer(buffer, buffSize, node->dirName, filename);

	free(buffer);

	return ret;
}

inline int getNumProc(struct MPINode *node)
{
	return node->numProc;
}

inline int getNodeId(struct MPINode *node)
{
	return node->ownId;
}

void statsAvg(struct Stats *outStats, struct MPINode *node)
{
	int i;
	double *sendBuff;
	double *recvBuff, *recvP;
	size_t sendCount = 6 + node->stats->nThreads;
	size_t recvCount = sendCount * node->numProc;

	// Allocate buffers
	sendBuff = (double *)mallocC(sendCount * sizeof(double));
	recvBuff = (double *)mallocC(recvCount * sizeof(double));
	recvP = recvBuff;

	// Prepare send buffer
	sendBuff[0] = node->stats->total;
	sendBuff[1] = node->stats->mpiIteration;
	sendBuff[2] = node->stats->communication;
	sendBuff[3] = node->stats->ompIteration;
	sendBuff[4] = node->stats->cellChecking;
	sendBuff[5] = node->stats->worldUpdate;
	for (i = 0; i < node->stats->nThreads; ++i)
		sendBuff[6 + i] = node->stats->threads[i];

	// Receive all stats
	MPI_Gather(
		sendBuff, sendCount, MPI_DOUBLE,
		recvBuff, sendCount, MPI_DOUBLE,
		0, MPI_COMM_WORLD
	);

	// Calculate average
	outStats->total         = 0;
	outStats->mpiIteration  = 0;
	outStats->communication = 0;
	outStats->ompIteration  = 0;
	outStats->cellChecking  = 0;
	outStats->worldUpdate   = 0;
	for (i = 0; i < node->stats->nThreads; ++i)
		outStats->threads[i] = 0;

	if (node->ownId == 0) {
		while(recvCount) {
			outStats->total         += recvP[0];
			outStats->mpiIteration  += recvP[1];
			outStats->communication += recvP[2];
			outStats->ompIteration  += recvP[3];
			outStats->cellChecking  += recvP[4];
			outStats->worldUpdate   += recvP[5];
			for (i = 0; i < node->stats->nThreads; ++i)
				outStats->threads[i] += recvP[6 + i];

			recvP += sendCount;
			recvCount -= sendCount;
		}
	}

	outStats->total         /= 2.0;
	outStats->mpiIteration  /= 2.0;
	outStats->communication /= 2.0;
	outStats->ompIteration  /= 2.0;
	outStats->cellChecking  /= 2.0;
	outStats->worldUpdate   /= 2.0;
	for (i = 0; i < node->stats->nThreads; ++i)
		outStats->threads[i] /= 2.0;

	free(sendBuff);
	free(recvBuff);
}
