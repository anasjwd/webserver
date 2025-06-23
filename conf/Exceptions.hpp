#pragma once

#include <exception>

class DirectiveException : public std::exception {
	private:
		const char* msg;
		DirectiveException(void);

	public:
		DirectiveException(const char* msg);
		const char* what() const throw();
};

class IncompleteConfig : public std::exception {
	public:
		const char* what() const throw();
};
