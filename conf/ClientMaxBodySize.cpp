#pragma once

#include "cfg_parser.hpp"

ClientMaxBodySize::ClientMaxBodySize(unsigned int size) : size(size)
{}

ClientMaxBodySize::~ClientMaxBodySize(void)
{}

DIRTYPE ClientMaxBodySize::getType(void) const
{
	return CLIENT_MAX_BODY_SIZE;
}

