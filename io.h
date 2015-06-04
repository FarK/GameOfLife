#ifndef IO_H_
#define IO_H_

// TODO: Make it cross-platform
#include <stdio.h>
#include <stdbool.h>

// TOOD: Make it cross-platform
bool createSubdir(const char *dirName);
bool writeBuffer(char *buffer, size_t size, const char *dirName,
	const char *filename, const char *mode);

#endif
