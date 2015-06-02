#ifndef STATS_H_
#define STATS_H_

#include <stdbool.h>

struct Stats;


struct Stats *createStats(unsigned long long int iterations, int nThreads);
void freeStats(struct Stats *stats);

double startMeasurement();
double endMeasurement(double startTime);

void addIterationTime(double time, struct Stats *stats);
void addCommunicationTime(double time, struct Stats *stats);
void addSubIterationTime(double time, struct Stats *stats);
void addThreadTime(double time, int threadNum, struct Stats *stats);

bool saveStats(struct Stats *stats);

#endif
