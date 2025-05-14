#pragma once

#include "cfg_parser.hpp"

Http::Http(void)
{
}

Http::~Http(void)
{
	unsigned int size;

	size = directives.size();
	for (unsigned int i = 0; i < size; i++)
		delete directives[i];
}

DIRTYPE Http::getType(void) const
{
	return HTTP;
}

void Http::addDirective(IDirective* dir)
{
	directives.push_back(dir);
}
