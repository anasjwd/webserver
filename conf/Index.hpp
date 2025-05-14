#pragma once

#include "IDirective.hpp"

class Index : public IDirective {
	private:
		char** files;
		char* pathToLastFile;

		Index(void);
		Index(const Index& other);
		Index& operator=(const Index& other);

	public:
		Index(char** files, char* pathToLastFile);
		~Index(void);
		DIRTYPE getType(void) const;
};

