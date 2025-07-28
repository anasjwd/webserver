#pragma once

#include "IDirective.hpp"

class Return : public IDirective {
	private:
		unsigned int code;
		char* url;

		Return(const Return& other);
		Return& operator=(const Return& other);

	public:
		Return(void);
		~Return(void);
		DIRTYPE getType(void) const;
		void setCode(unsigned int value);
		void setUrl(char* value);
		unsigned int getCode() const { return code; }
		char* getUrl() const { return url; }
};

