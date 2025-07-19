#pragma once

#include "IDirective.hpp"

class Deny : public IDirective {
	private:
		char* denied;

		Deny(const Deny& other);
		Deny& operator=(const Deny& other);

	public:
		Deny(void);
		~Deny(void);
		DIRTYPE getType(void) const;
		void setDenied(char* value);
		char* getDenied(void) const;
};

