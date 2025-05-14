#pragma once

#include "IDirective.hpp"

class Listen : public IDirective {
	private:
		char* host;
		unsigned int port;
		Listen(void);

	public:
		Listen(char* host, unsigned int port);
		~Listen(void);
		Listen(const Listen& other);
		Listen& operator=(const Listen& other);
		DIRTYPE getType(void) const;
};

