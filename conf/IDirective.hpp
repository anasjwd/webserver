#pragma once

#include "cfg_parser.hpp"

class IDirective {
	public:
		virtual DIRTYPE getType(void) const = 0;
};

