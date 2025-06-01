#include "ErrorPage.hpp"

ErrorPage::ErrorPage(void)
{
	uri = NULL;
}

ErrorPage::~ErrorPage(void)
{
	free(uri);
}

DIRTYPE ErrorPage::getType(void) const
{
	return ERROR_PAGE;
}

void ErrorPage::setCode(int value)
{
	code = value;
}

void ErrorPage::setResponseCode(int value)
{
	responseCode = value;
}

void ErrorPage::setUri(char* value)
{
	uri = value;
}

int ErrorPage::getCode(void) const
{
	return code;
}

int ErrorPage::getResponseCode(void) const
{
	return responseCode;
}

char* ErrorPage::getUri(void) const
{
	return uri;
}
