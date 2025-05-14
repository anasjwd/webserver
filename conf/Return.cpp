#pragma once

#include "cfg_parser.hpp"

Return::Return(unsigned int code, char* url) : code(code), url(url)
{}

Return::~Return(void)
{
	delete[] url;
}

DIRTYPE Return::getType(void) const
{
	return RETURN;
}

