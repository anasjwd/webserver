#pragma once

#include "IDirective.hpp"
#include <cstdlib>

class Index : public IDirective {
	private:
		char** files;

		Index(const Index& other);
		Index& operator=(const Index& other);

	public:
		Index(void);
		~Index(void);
		DIRTYPE getType(void) const;
		void setFiles(char** value);
		void setFile(char* value, unsigned int idx);
		char** getFiles() const;
};

