#pragma once

#include "IDirective.hpp"

class ErrorPage : public IDirective {
	private:
		unsigned int code;
		unsigned int responseCode;
		char* uri;

		ErrorPage(void);
		ErrorPage(const ErrorPage& other);
		ErrorPage operator=(const ErrorPage& other);

	public:
		ErrorPage(unsigned int code, unsigned int responseCode, char* uri);
		~ErrorPage(void);
		DIRTYPE getType(void) const;
};
