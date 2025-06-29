#include "LimitExcept.hpp"

LimitExcept::LimitExcept(void)
{
	methods = NULL;
}

LimitExcept::~LimitExcept(void)
{
	for (unsigned int i = 0; methods[i] != NULL; i++)
		free(methods[i]);
	delete[] methods;
}

DIRTYPE LimitExcept::getType(void) const
{
	return LIMIT_EXCEPT;
}

void LimitExcept::setMethods(char** value)
{
	methods = value;
}

void LimitExcept::setMethod(char* value, unsigned int idx)
{
	methods[idx] = value;
}

bool LimitExcept::validate(void)
{
	unsigned int size = directives.size();

	for (unsigned int i = 0; i < size; i++)
	{
		if (directives[i]->getType() != ALLOW
				&& directives[i]->getType() != DENY)
		{
			std::cerr << "Error: Invlaid directive inside of limit_except block\n";
			return ( false );
		}
	}
	return ( true );
}
