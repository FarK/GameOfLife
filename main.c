#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>
#include "node.h"
#include "stats.h"
#include <omp.h>

bool processArgs(struct Parameters *params, int argc, char *argv[]);
void printHelp(char *argv[]);
void poblateWorld(struct MPINode *node, struct Parameters *params);

void treadIOError(struct MPINode *node);

int main(int argc, char *argv[])
{
	struct MPINode *node;
	struct Parameters params;
	struct Stats *stats, *avgStats;

	srand(time(NULL));

	if(!processArgs(&params, argc, argv))
		return EXIT_FAILURE;

	MPI_Init(NULL, NULL);

	stats = createStats(params.iterations, params.numThreads);
	avgStats = createStats(params.iterations, params.numThreads);
	node = createNode(&params, stats);

	poblateWorld(node, &params);

	run(node);

	statsAvg(avgStats, node);
	if (getNodeId(node) == 0) {
		saveStats(avgStats);
	}

	freeStats(stats);
	freeStats(avgStats);
	deleteNode(node);
	MPI_Finalize();

	return EXIT_SUCCESS;
}

void poblateWorld(struct MPINode *node, struct Parameters *params)
{
	int nodeId = getNodeId(node);
	int numProc = getNumProc(node);
	wsize_t x, y;
	long long unsigned int i;

	if (params->cells == 0) {
		if (nodeId == 0) {
			// Glider pattern
			node_reviveCell(2,7, node);
			node_reviveCell(2,8, node);
			node_reviveCell(2,9, node);
			node_reviveCell(3,7, node);
			node_reviveCell(4,8, node);
		}
	} else {
		for (i = 0; i < params->cells; ++i) {
			if (nodeId == rand()%numProc) {
				x = rand()%params->x;
				y = rand()%params->y;

				node_reviveCell(x,y, node);
			}
		}
	}
}

bool processArgs(struct Parameters *params, int argc, char *argv[])
{
	static int record;

	static struct option options[] =
	{
		{"size",       required_argument, NULL,    's'},
		{"threads",    required_argument, NULL,    't'},
		{"iterations", required_argument, NULL,    'i'},
		{"cells",      optional_argument, NULL,    'c'},
		{"record",     no_argument,       &record,  1 },
		{0, 0, 0, 0}
	};

	int optIdx;
	int opt;
	char *x_char;
	char *pEnd;

	params->x = 0;
	params->y = 0;
	params->numThreads = -1;
	params->iterations = 0;
	params->cells = 0;

	while(1) {
		opt = getopt_long(argc, argv, "s:t:i:c::r", options, &optIdx);
		if (opt == -1) break;

		switch (opt) {
			case 0:
				break;
			case 's':
				x_char = strpbrk(optarg, "x");
				if (x_char == NULL) goto error;
				*x_char = ' ';

				params->x = (wsize_t)strtol(optarg, &pEnd, 10);
				if (errno == ERANGE) goto error;
				params->y = (wsize_t)strtol(pEnd, NULL, 10);
				if (errno == ERANGE) goto error;
				break;

			case 't':
				params->numThreads =
					(int)strtol(optarg, NULL, 10);
				if (errno == ERANGE) goto error;
				break;

			case 'i':
				params->iterations =
					(long long int)strtol(optarg, NULL, 10);
				if (errno == ERANGE) goto error;
				break;

			case 'c':
				params->cells =
					(long long int)strtol(optarg, NULL, 10);
				if (errno == ERANGE) goto error;
				break;

			case 'r':
				record = 1;
				break;
			case '?':
			default:
				goto error;
		};
	}

	params->record = record;

	if (params->numThreads == 0) params->numThreads = omp_get_max_threads();

	if (
		params->x == 0 ||
		params->y == 0 ||
		params->numThreads == -1 ||
		params->iterations == 0
	)
		goto error;

	return true;

error:	printHelp(argv);
	return false;
}

void printHelp(char *argv[])
{
	fprintf(stderr,
		"Usage: %s "
		"--size <x>x<y> "
		"--threads <number> "
		"--iterations <number> "
		"[--cells <number>] "
		"[--record]"
		"\n",
		argv[0]
	);

	fprintf(stderr, "\t-s, --size <x size>x<y size>\n");
	fprintf(stderr, "\t\tTotal size of the world (Ex: -s100x100)\n\n");

	fprintf(stderr, "\t-t, --threads <num of threads>\n");
	fprintf(stderr, "\t\tNumber of thread to use. If 0, max possible threads will be selected\n\n");

	fprintf(stderr, "\t-i, --iterations <number of iterations>\n");
	fprintf(stderr, "\t\tNumber of iteratons to do\n\n");

	fprintf(stderr, "\t-c, --cells <number of cells>\n");
	fprintf(stderr, "\t\tNumber of random cells to create. If it is not set, a glider patter will be set\n\n");

	fprintf(stderr, "\t-r, --record\n");
	fprintf(stderr, "\t\tSave each iterations. CAUTION: Do not use with bigs worlds\n\n");
}
