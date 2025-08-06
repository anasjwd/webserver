#pragma once

#include "IDirective.hpp"

class Root : public IDirective {
	private:
		char* path;

		Root(const Root& other);
		Root& operator=(const Root& other);

	public:
		Root(void);
		~Root(void);
		DIRTYPE getType(void) const;
		void setPath(char* value);
		char* getPath(void) const;
};
