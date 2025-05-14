#pragma once

#include "cfg_parser.hpp"

ServerName::ServerName(char** serverNames) : serverNames(serverNames)
{}

ServerName::~ServerName(void)
{
	for (unsigned int i = 0; serverNames[i] != NULL; i++)
		delete[] serverNames[i];
	delete[] serverNames;
}

DIRTYPE ServerName::getType(void) const
{
	return SERVER_NAME;
}

