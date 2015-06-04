#include "io.h"
#include "malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

static bool rmRDir(const char *dirName);

bool createSubdir(const char *dirName)
{
	int mkdirRet;

	rmRDir(dirName);
	mkdirRet = mkdir(dirName, 0775);
	return mkdirRet == 0 || errno != EEXIST;
}

bool writeBuffer(char *buffer, size_t size, const char *dirName,
	const char *filename, const char *mode)
{
	FILE *file;
	size_t written;
	char *route;
	size_t routeLenght;

	routeLenght = strlen(dirName) + strlen(filename) + 2;
	route = (char *)mallocC(routeLenght * sizeof(char));

	snprintf(route,  routeLenght, "%s/%s", dirName, filename);

	file = fopen(route, mode);
	if (file == NULL) return false;

	written = fwrite(buffer, sizeof(char), size, file);
	if (written != size) return false;

	free(route);
	fclose(file);

	return true;
}

static bool rmRDir(const char *dirName)
{
	DIR *dir;
	struct dirent *entity;
	struct stat attrib;
	size_t dirNameSize, nameSize;
	char *filename;

	dirNameSize = strlen(dirName);

	dir = opendir(dirName);
	if (!dir) return false;

	while ((entity = readdir(dir)) != NULL) {
		if (strcmp(entity->d_name, ".") == 0 ||
		    strcmp(entity->d_name, "..") == 0
		) continue;

		nameSize = dirNameSize + strlen(entity->d_name) + 2;
		filename = (char *)mallocC(nameSize);
		snprintf(filename, nameSize, "%s/%s", dirName, entity->d_name);

		if (stat(filename, &attrib) == -1)
			continue;

		if (S_ISDIR(attrib.st_mode))
			rmRDir(filename);
		else
			unlink(filename);

		free(filename);
	}

	closedir(dir);

	return rmdir(dirName);
}
