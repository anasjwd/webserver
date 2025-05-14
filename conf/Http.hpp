#pragma once

#include "IDirective.hpp"

class Http : public IDirective {
	private:
		std::vector<IDirective*> directives;
		Http(const Http& other);
		Http& operator=(const Http& other);
	public:
		Http(void);
		~Http(void);
		DIRTYPE getType(void) const;
		void addDirective(IDirective* dir);
};
