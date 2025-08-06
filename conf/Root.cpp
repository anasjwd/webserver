#include "Root.hpp"

Root::Root(void) {
	path = NULL;
}

Root::~Root(void) {
	free(path);
}

DIRTYPE Root::getType(void) const {
	return ROOT;
}

void Root::setPath(char* value) {
	path = value;
}

char* Root::getPath(void) const {
	return ( path );
}
