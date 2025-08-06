#pragma once

#include "IDirective.hpp"

class UploadLocation : public IDirective {
	private:
		char* location;

		UploadLocation(const UploadLocation& other);
		UploadLocation& operator=(const UploadLocation& other);

	public:
		UploadLocation(void);
		~UploadLocation(void);

		DIRTYPE getType(void) const;
		void setLocation(char* value);
		char* getLocation(void) const;
};
