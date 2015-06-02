#include "stats.h"
#include "io.h"
#include <stdlib.h>
#include <omp.h>

#define _TO_STR(val) #val
#define TO_STR(val) _TO_STR(val)

#define STRLEN(str) (sizeof(str)/sizeof(str[0]) - 1)

#define INT_DIG 4
#define DEC_DIG 10
#define DIGS (INT_DIG + DEC_DIG + 1)
#define PF_FORM "%"TO_STR(INT_DIG)"."TO_STR(DEC_DIG)"f"

struct Stats {
	double avgFactor;
	int nThreads;

	double tProccess;
	double tIteration;
	double tComunication;
	double tSubIteration;
	double *tThreads;
};

static void addMeasurement(double *avg, double time,
	struct Stats *stats);

struct Stats *createStats(unsigned long long int iterations, int nThreads)
{
	int i;
	struct Stats *stats;

	stats = (struct Stats *)malloc(sizeof(struct Stats));
	stats->tThreads = (double *)malloc(nThreads * sizeof(double));

	stats->avgFactor = 1.0/(double)iterations;
	stats->nThreads = nThreads;
	stats->tProccess = 0.0;
	stats->tIteration = 0.0;
	stats->tComunication = 0.0;
	stats->tSubIteration = 0.0;

	for (i = 0; i < nThreads; ++i)
		stats->tThreads[i] = 0.0;

	return stats;
}

void freeStats(struct Stats *stats)
{
	free(stats->tThreads);
	free(stats);
}

inline double startMeasurement()
{
	return omp_get_wtime();
}

inline double endMeasurement(double startTime)
{
	return omp_get_wtime() - startTime;
}

inline static void addMeasurement(double *avg, double time,
	struct Stats *stats)
{
	*avg = *avg + stats->avgFactor*time;
}

inline void addProccessTime(double time, struct Stats *stats)
{
	addMeasurement(&stats->tProccess, time, stats);
}

inline void addIterationTime(double time, struct Stats *stats)
{
	addMeasurement(&stats->tIteration, time, stats);
}

inline void addCommunicationTime(double time, struct Stats *stats)
{
	addMeasurement(&stats->tComunication, time, stats);
}

inline void addSubIterationTime(double time, struct Stats *stats)
{
	addMeasurement(&stats->tSubIteration, time, stats);
}

inline void addThreadTime(double time, int threadNum, struct Stats *stats)
{
	addMeasurement(&stats->tThreads[threadNum], time, stats);
}

bool saveStats(struct Stats *stats)
{
	int i;
	char *buffer, *pBuffer;
	size_t maxBuffSize, maxLineSize;
	int written;

	maxLineSize = STRLEN("Communications\t\t" "\n") + DIGS;
	maxBuffSize = (3 + stats->nThreads)*maxLineSize;
	buffer = (char *)malloc(maxBuffSize * sizeof(char));
	pBuffer = buffer;

	written = snprintf(pBuffer, maxBuffSize,
		"Iterations\t\t" PF_FORM "\n"
		"Communications\t\t" PF_FORM "\n"
		"SubIteration\t\t" PF_FORM "\n",
		stats->tIteration,
		stats->tComunication,
		stats->tSubIteration
	);
	pBuffer = buffer + written;

	for (i = 0; i < stats->nThreads; ++i) {
		written += snprintf(pBuffer, maxBuffSize - written,
			"Thread%d\t\t" PF_FORM "\n",
			i,
			stats->tThreads[i]
		);
		pBuffer = buffer + written;
	}

	writeBuffer(buffer, written, "./", "stats");
	free(buffer);

	return true;
}
