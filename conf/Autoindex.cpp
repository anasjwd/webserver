#pragma once

#include "cfg_parser.hpp"

Autoindex::Autoindex(bool state) : state(state)
{}

Autoindex::~Autoindex(void)
{}

DIRTYPE Autoindex::getType(void) const
{
	return AUTOINDEX;
}

