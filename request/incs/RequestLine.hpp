# pragma once

# include <map>
# include <string>
# include "Defines.hpp"

class	RequestLine
{
	private:
		std::string				_raw;
		std::string				_uri;
		std::string				_method;
		std::string				_version;
		HttpStatusCode			_statusCode;

		std::map<std::string, std::string>	_queryParams;

		bool					_validateUri();
		bool					_validateMethod();
		bool					_validateVersion();
		void					_parseQueryString();

	public:
		RequestLine(const std::string&);

		void					clear();
		bool					parse();

		const std::string&		getUri() const;
		const std::string&		getMethod() const;
		const std::string&		getVersion() const;

		HttpStatusCode			getStatusCode() const;
		bool					setState(bool, HttpStatusCode);

		const std::map<std::string, std::string>&	getQueryParams() const;
};
