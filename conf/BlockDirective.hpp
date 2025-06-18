#pragma once

#include "IDirective.hpp"
#include <vector>

class BlockDirective : public IDirective {
	private:
		BlockDirective(const BlockDirective& other);
		BlockDirective& operator=(const BlockDirective& other);

	public:
		std::vector<IDirective*> directives;
		BlockDirective(void);
		~BlockDirective(void);
		virtual void addDirective(IDirective* dir);
};
