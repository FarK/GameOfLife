#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>
#include "node.h"

struct Parameters {
	wsize_t x, y;
	int numThreads;
	long long unsigned int iterations;
	int record;
};

bool processArgs(struct Parameters *params, int argc, char *argv[]);
void printHelp(char *argv[]);
void poblateWorld(struct MPINode *node);

void treadIOError(struct MPINode *node);

int main(int argc, char *argv[])
{
	struct MPINode *node;
	struct Parameters params;

	if(!processArgs(&params, argc, argv))
		return EXIT_FAILURE;

	node = createNode(params.x, params.y, params.numThreads);
	if (node == NULL) treadIOError(node);

	poblateWorld(node);

	while (params.iterations--) {
		iterate(node);

		if (params.record) {
			if (!write(node)) treadIOError(node);
		}
	}
	if (!write(node)) treadIOError(node);

	deleteNode(node);

	return EXIT_SUCCESS;
}

void poblateWorld(struct MPINode *node)
{
	int nodeId = getNodeId(node);

	if (nodeId == 0) {
		node_reviveCell(2,7, node);
		node_reviveCell(2,8, node);
		node_reviveCell(2,9, node);
		node_reviveCell(3,7, node);
		node_reviveCell(4,8, node);
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

	while(1) {
		opt = getopt_long(argc, argv, "s:t:i:r", options, &optIdx);
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
					(int)strtol(optarg, NULL, 10);
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
		"Usage: %s --size <x size>x<y size> --threads <num of threads> --iterations <number of iterations>\n",
		argv[0]
	);

	fprintf(stderr, "\t-s, --size <x size>x<y size>\n");
	fprintf(stderr, "\t\tTotal size of the world (Ex: -s100x100)\n\n");

	fprintf(stderr, "\t-t, --threads <num of threads>\n");
	fprintf(stderr, "\t\tNumber of thread to use. If 0, max possible threads will be selected\n\n");

	fprintf(stderr, "\t-i, --iterations <number of iterations>\n");
	fprintf(stderr, "\t\tNumber of iteratons to do\n");
}

void treadIOError(struct MPINode *node)
{
	fprintf(stderr,
		"Can't write output. Please make sure you have enough permmissions\n"
	);
	nodeAbort(node);

	exit(EXIT_FAILURE);
}

