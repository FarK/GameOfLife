#ifndef NODE_H_
#define NODE_H_

#include "world.h"
#include "types.h"

struct MPINode;

struct MPINode *createNode(wsize_t ws_x, wsize_t ws_y);
void deleteNode(struct MPINode *node);
void iterate(struct MPINode *node);
void node_reviveCell(wsize_t x, wsize_t y, struct MPINode *node);
void node_killCell(wsize_t x, wsize_t y, struct MPINode *node);

void printNode(struct MPINode *node);

#endif
