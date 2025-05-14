#pragma once

#include "IDirective.hpp"

class Location : public IDirective {
	private:
		char* uri;
		bool exactMatch;
		std::vector<IDirective*> directives;

		Location(void);
		Location(const Location& other);
		Location& operator=(const Location& other);

	public:
		Location(char* uri, bool exactMatch);
		~Location(void);
		DIRTYPE getType(void) const;
		void addDirective(IDirective* dir);
};

