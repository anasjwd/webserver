#include "Upload.hpp"

Upload::Upload(void): state(false) {}

Upload::~Upload(void) {}

DIRTYPE Upload::getType(void) const {
	return UPLOAD;
}

void Upload::setState(bool value) {
	state = value;
}

bool Upload::getState(void) const {
	return ( state );
}
