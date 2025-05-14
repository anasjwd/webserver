#pragma once

#include "cfg_parser.hpp"

Root::Root(char* path) : path(path)
{}

Root::~Root(void)
{
	delete[] path;
}

DIRTYPE Root::getType(void) const
{
	return ROOT;
}

