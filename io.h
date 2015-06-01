#ifndef IO_H_
#define IO_H_

// TODO: Make it cross-platform
#include <stdio.h>
#include <stdbool.h>

#define MAX_FILENAME 10

// TOOD: Make it cross-platform
bool createSubdir(char *dirName);
bool writeBuffer(char *buffer, size_t size, char *dirName, char *filename);

#endif
