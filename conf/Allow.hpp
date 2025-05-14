#pragma once

#include "IDirective.hpp"

class Allow : public IDirective {
	private:
		char* allowed;

		Allow(void);
		Allow(const Allow& other);
		Allow& operator=(const Allow& other);

	public:
		Allow(char* allowed);
		~Allow(void);
		DIRTYPE getType(void) const;
};

