#pragma once

#include "IDirective.hpp"

class Upload : public IDirective {
	private:
		bool state;

		Upload(const Upload& other);
		Upload& operator=(const Upload& other);

	public:
		Upload(void);
		~Upload(void);

		DIRTYPE getType(void) const;
		void setState(bool value);
		bool getState(void) const;
};
