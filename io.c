#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

bool rmRDir(char *dirName);

bool createSubdir(char *dirName)
{
	int mkdirRet;

	rmRDir(dirName);
	mkdirRet = mkdir(dirName, 0775);
	return mkdirRet == 0 || errno != EEXIST;
}

bool writeBuffer(char *buffer, size_t size, char *dirName, char *filename)
{
	FILE *file;
	size_t written;
	char route[MAX_FILENAME*2 + 1];

	snprintf(route, MAX_FILENAME*2 + 1, "%s/%s", dirName, filename);

	file = fopen(route, "w");
	if (file == NULL) return false;

	written = fwrite(buffer, sizeof(char), size, file);
	if (written != size) return false;

	fclose(file);

	return true;
}

bool rmRDir(char *dirName)
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
		filename = (char *)malloc(nameSize);
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
