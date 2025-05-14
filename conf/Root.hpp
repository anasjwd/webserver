#pragma once

#include "IDirective.hpp"

class Root : public IDirective {
	private:
		char* path;

		Root(void);
		Root(const Root& other);
		Root& operator=(const Root& other);

	public:
		Root(char* path);
		~Root(void);
		DIRTYPE getType(void) const;
};

