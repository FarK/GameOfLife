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
