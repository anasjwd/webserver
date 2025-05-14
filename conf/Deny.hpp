#pragma once

#include "IDirective.hpp"

class Deny : public IDirective {
	private:
		char* denied;

		Deny(void);
		Deny(const Deny& other);
		Deny& operator=(const Deny& other);

	public:
		Deny(char* denied);
		~Deny(void);
		DIRTYPE getType(void) const;
};

