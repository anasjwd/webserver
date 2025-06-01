#pragma once

#include "BlockDirective.hpp"
#include <cstdlib>

class LimitExcept : public BlockDirective {
	private:
		char** methods;

		LimitExcept(const LimitExcept& other);
		LimitExcept& operator=(const LimitExcept& other);

	public:
		LimitExcept(void);
		~LimitExcept(void);
		DIRTYPE getType(void) const;
		void setMethods(char** value);
		void setMethod(char* value, unsigned int idx);
};

