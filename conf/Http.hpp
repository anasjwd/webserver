#pragma once

#include "BlockDirective.hpp"

class Http : public BlockDirective {
	private:
		Http(const Http& other);
		Http& operator=(const Http& other);
	public:
		Http(void);
		~Http(void);
		DIRTYPE getType(void) const;
		bool validate(void);
};
