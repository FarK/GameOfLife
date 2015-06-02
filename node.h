#ifndef NODE_H_
#define NODE_H_

#include "world.h"

struct Parameters {
	wsize_t x, y;
	int numThreads;
	long long unsigned int iterations;
	int record;
};

struct MPINode;

struct MPINode *createNode(const struct Parameters *params);
void deleteNode(struct MPINode *node);
void nodeAbort(struct MPINode *node);
void run(struct MPINode *node);
void node_reviveCell(wsize_t x, wsize_t y, struct MPINode *node);
void node_killCell(wsize_t x, wsize_t y, struct MPINode *node);
int getNodeId(struct MPINode *node);
bool write(struct MPINode *node);

#endif
