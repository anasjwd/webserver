#pragma once

#include "IDirective.hpp"

class AutoIndex : public IDirective {
	private:
		bool state;

		AutoIndex(const AutoIndex& other);
		AutoIndex& operator=(const AutoIndex& other);

	public:
		AutoIndex(void);
		~AutoIndex(void);
		DIRTYPE getType(void) const;
		void setState(bool value);
		bool getState() const;
};

