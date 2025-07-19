#pragma once

#include "IDirective.hpp"
#include <vector>
#include <iostream>

class BlockDirective : public IDirective {
	private:
		BlockDirective(const BlockDirective& other);
		BlockDirective& operator=(const BlockDirective& other);

	public:
		std::vector<IDirective*> directives;
		BlockDirective(void);
		~BlockDirective(void);
		virtual void addDirective(IDirective* dir);
		virtual bool validate(void);
		IDirective* getDirective(DIRTYPE type) const;
};
