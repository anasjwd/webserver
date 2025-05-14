#pragma once

#include "cfg_parser.hpp"

LimitExcept::LimitExcept(char** methods) : methods(methods)
{}

LimitExcept::~LimitExcept(void)
{
	for (unsigned int i = 0; methods[i] != NULL; i++)
		delete[] methods[i];
	delete[] methods[i];
}

DIRTYPE LimitExcept::getType(void) const
{
	return LIMIT_EXCEPT;
}

void LimitExcept::addDirective(IDirective* dir)
{
	directives.push_back(dir);
}
