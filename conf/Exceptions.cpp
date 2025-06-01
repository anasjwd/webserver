#include "Exceptions.hpp"

DirectiveException::DirectiveException(const char* msg) : msg(msg) {}

const char* DirectiveException::what() const throw()
{
	return ( msg );
}

const char* IncompleteConfig::what() const throw()
{
	return "Incomplete configuration";
}

