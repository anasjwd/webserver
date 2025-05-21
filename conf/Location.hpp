#pragma once

#include "IDirective.hpp"

class Location : public IDirective {
	private:
		char* uri;
		bool exactMatch;
		std::vector<IDirective*> directives;

		Location(const Location& other);
		Location& operator=(const Location& other);

	public:
		Location(void);
		~Location(void);
		DIRTYPE getType(void) const;
		void addDirective(IDirective* dir);
};

