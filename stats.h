#ifndef STATS_H_
#define STATS_H_

#include <stdbool.h>

struct Stats {
	double avgFactor;
	int nThreads;

	double total;
	double mpiIteration;
	double communication;
	double ompIteration;
	double cellChecking;
	double worldUpdate;
	double *threads;
};


struct Stats *createStats(unsigned long long int iterations, int nThreads);
void freeStats(struct Stats *stats);

#define startMeasurement() omp_get_wtime()

#define endMeasurement(time, stName, stats)\
	(stats)->stName = (stats)->stName + (stats)->avgFactor*(omp_get_wtime()-(time))

bool saveStats(struct Stats *stats);
bool saveStatsGnuplot(struct Stats *stats);

#endif
