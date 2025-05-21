#pragma once

#include "cfg_parser.hpp"

Server::Server()
{}

Server::~Server(void)
{
	unsigned int size;

	size = directives.size();
	for (unsigned int i = 0; i < size; i++)
		delete directives[i];
}

DIRTYPE Server::getType(void) const
{
	return SERVER;
}

void Server::addDirective(IDirective* dir)
{
	directives.puch_back(dir);
}
