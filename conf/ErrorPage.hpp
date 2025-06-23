#pragma once

#include "IDirective.hpp"

class ErrorPage : public IDirective {
	private:
		int code;
		int responseCode;
		char* uri;

		ErrorPage(const ErrorPage& other);
		ErrorPage operator=(const ErrorPage& other);

	public:
		ErrorPage(void);
		~ErrorPage(void);
		DIRTYPE getType(void) const;
		void setCode(int value);
		void setResponseCode(int value);
		void setUri(char* value);
		int getCode(void) const;
		int getResponseCode(void) const;
		char* getUri(void) const;
};
