#pragma once

#include "IDirective.hpp"j

class Return : public IDirective {
	private:
		unsigned int code;
		char* url;

		Return(void);
		Return(const Return& other);
		Return& operator=(const Return& other);

	public:
		Return(unsigned int code, char* url);
		~Return(void);
		DIRTYPE getType(void) const;
};

