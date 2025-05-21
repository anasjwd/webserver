#pragma once

#include "cfg_parser.hpp"

Location::Location()
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
