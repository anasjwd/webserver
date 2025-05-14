#pragma once

#include "cfg_parser.hpp"

Location::Location(char* uri, bool exactMatch) :
	uri(uri),
	exactMatch(exactMatch)
{}

Location::~Location(void)
{
	delete[] uri;
}

DIRTYPE Location::getType(void) const
{
	return LOCATION;
}

void Location::addDirective(IDirective* dir)
{
	directives.push_back(dir);
}
