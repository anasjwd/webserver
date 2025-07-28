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

bool BlockDirective::validate(void)
{
	return ( true );
}

IDirective* BlockDirective::getDirective(DIRTYPE type) const
{
	for (std::vector<IDirective*>::const_iterator it = directives.begin(); it != directives.end(); ++it) {
		if ((*it)->getType() == type)
			return *it;
	}
	return NULL;
}
