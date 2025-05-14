#pragma once

#include "IDirective.hpp"

class AutoIndex : public IDirective {
	private:
		bool state;

		AutoIndex(void);
		AutoIndex(const AutoIndex& other);
		AutoIndex& operator=(const AutoIndex& other);

	public:
		AutoIndex(bool state);
		~AutoIndex(void);
		DIRTYPE getType(void) const;
};

