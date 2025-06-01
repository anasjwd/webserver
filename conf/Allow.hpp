#pragma once

#include "IDirective.hpp"

class Allow : public IDirective {
	private:
		char* allowed;

		Allow(const Allow& other);
		Allow& operator=(const Allow& other);

	public:
		Allow(void);
		~Allow(void);
		DIRTYPE getType(void) const;
		void setAllowed(char* value);
};

