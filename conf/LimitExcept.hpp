#pragma once

#include "IDirective.hpp"

class LimitExcept : public IDirective {
	private:
		char** methods;
		std::vector<IDirective*> directives;

		LimitExcept(void);
		LimitExcept(const LimitExcept& other);
		LimitExcept& operator=(const LimitExcept& other);

	public:
		LimitExcept(char** methods);
		~LimitExcept(void);
		DIRTYPE getType(void) const;
		void addDirective(IDirective* dir);
};

