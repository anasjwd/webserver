#pragma once

#include "IDirective.hpp"
#include <vector>

class BlockDirective : public IDirective {
	private:
		BlockDirective(const BlockDirective& other);
		BlockDirective& operator=(const BlockDirective& other);

	protected:
		std::vector<IDirective*> directives;

	public:
		BlockDirective(void);
		~BlockDirective(void);
		virtual void addDirective(IDirective* dir);
};
