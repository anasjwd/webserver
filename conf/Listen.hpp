#pragma once

#include "IDirective.hpp"

class Listen : public IDirective {
	private:
		char* host;
		unsigned short port;
		Listen(const Listen& other);
		Listen& operator=(const Listen& other);

	public:
		Listen(void);
		~Listen(void);

		DIRTYPE getType(void) const;
		void setHost(char* value);
		void setPort(unsigned int value);
		unsigned int getPort(void) const;
		char* getHost(void) const;
};

