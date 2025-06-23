#include "BlockDirective.hpp"

BlockDirective::BlockDirective(void)
{}

BlockDirective::~BlockDirective(void)
{
	unsigned int size = directives.size();

	for (unsigned int i = 0; i < size; i++)
		delete directives[i];
}

void BlockDirective::addDirective(IDirective* dir)
{
	directives.push_back(dir);
}
