#include "UploadLocation.hpp"

UploadLocation::UploadLocation(void) {
	location = NULL;
}

UploadLocation::~UploadLocation(void) {
	free(location);
}

DIRTYPE UploadLocation::getType(void) const {
	return UPLOAD_LOCATION;
}

void UploadLocation::setLocation(char* value) {
	location = value;
}

char* UploadLocation::getLocation(void) const {
	return ( location );
}
