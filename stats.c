#include "stats.h"
#include "io.h"
#include <stdlib.h>
#include <omp.h>

#define _TO_STR(val) #val
#define TO_STR(val) _TO_STR(val)

#define STRLEN(str) (sizeof(str)/sizeof(str[0]) - 1)

#define DEC_DIG 10
#define DIGS (STRLEN("9.") + DEC_DIG + STRLEN("e+99"))
#define PF_FORM "%."TO_STR(DEC_DIG)"e"


struct Stats *createStats(unsigned long long int iterations, int nThreads)
{
	int i;
	struct Stats *stats;

	stats = (struct Stats *)malloc(sizeof(struct Stats));
	stats->threads = (double *)malloc(nThreads * sizeof(double));

	stats->avgFactor = 1.0/(double)iterations;
	stats->nThreads = nThreads;

	stats->total = 0.0;
	stats->mpiIteration = 0.0;
	stats->communication = 0.0;
	stats->ompIteration = 0.0;
	stats->cellChecking = 0.0;
	stats->worldUpdate = 0.0;

	for (i = 0; i < nThreads; ++i)
		stats->threads[i] = 0.0;

	return stats;
}

void freeStats(struct Stats *stats)
{
	free(stats->threads);
	free(stats);
}

bool saveStats(struct Stats *stats)
{
	int i;
	char *buffer, *pBuffer;
	size_t maxBuffSize, maxLineSize;
	int written;

	maxLineSize = STRLEN("            Thread9      \n") + DIGS;
	maxBuffSize = (6 + stats->nThreads)*maxLineSize + 1;
	buffer = (char *)malloc(maxBuffSize * sizeof(char));
	pBuffer = buffer;

	written = snprintf(pBuffer, maxBuffSize,
		"Total                    " PF_FORM "\n"
		"   MPI Iteration         " PF_FORM "\n"
		"      Communication      " PF_FORM "\n"
		"      OMP Iteration      " PF_FORM "\n"
		"         Cell checking   " PF_FORM "\n"
		"         World Update    " PF_FORM "\n",
		stats->total,
		stats->mpiIteration,
		stats->communication,
		stats->ompIteration,
		stats->cellChecking,
		stats->worldUpdate
	);
	pBuffer = buffer + written;

	for (i = 0; i < stats->nThreads; ++i) {
		written += snprintf(pBuffer, maxBuffSize - written,
			"            Thread%d      " PF_FORM "\n",
			i,
			stats->threads[i]
		);
		pBuffer = buffer + written;
	}

	writeBuffer(buffer, written, "./", "stats");
	free(buffer);

	return true;
}
