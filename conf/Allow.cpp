#pragma once

#include "cfg_parser.hpp"

Allow::Allow(char* allowed) : allowed(allowed)
{}

Allow::~Allow(void)
{
	delete[] allowed;
}

DIRTYPE Allow::getType(void) const
{
	return ALLOW;
}

