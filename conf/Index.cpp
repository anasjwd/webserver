#include "Index.hpp"
#include <iostream>

Index::Index(void)
{
	files = NULL;
}

Index::~Index(void)
{
	if (files != NULL) {
		for (unsigned int i = 0; files[i] != NULL; i++)
		{
			free(files[i]);
		}
		delete[] files;
	}
}

DIRTYPE Index::getType(void) const
{
	return INDEX;
}

void Index::setFiles(char** value)
{
	files = value;
}

void Index::setFile(char* value, unsigned int idx)
{
	files[idx] = value;
}

char** Index::getFiles() const
{
	return files;
}
