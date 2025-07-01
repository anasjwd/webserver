#include "LimitExcept.hpp"

LimitExcept::LimitExcept(void)
{
	methods = NULL;
}

LimitExcept::~LimitExcept(void)
{
	if (methods != NULL)
	{
		for (unsigned int i = 0; methods[i] != NULL; i++)
			free(methods[i]);
		delete[] methods;
	}
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
