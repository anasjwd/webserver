#pragma once

#include "cfg_parser.hpp"

Index::Index(char** files, char* pathToLastFile) :
	files(files),
	pathToLastFile(pathToLastFile)
{}

Index::~Index(void)
{
	for (unsigned int i = 0; files[i] != NULL; i++)
		delete[] files[i];
	delete[] files;
	delete[] pathToLastFile;
}

DIRTYPE Index::getType(void) const
{
	return INDEX;
}

