#pragma once

#include "cfg_parser.hpp"

ErrorPage::ErrorPage(unsigned int code, unsigned int responseCode, char* uri):
	code(code),
	responseCode(responseCode),
	uri(uri)
{}

ErrorPage::~ErrorPage(void)
{
	delete[] uri;
}

DIRTYPE ErrorPage::getType(void) const
{
	return ERROR_PAGE;
}

