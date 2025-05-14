#pragma once

#include "cfg_parser.hpp"

Deny::Deny(char* denied) : denied(denied)
{}

Deny::~Deny(void)
{
	delete[] denied;
}

DIRTYPE Deny::getType(void) const
{
	return DENY;
}

