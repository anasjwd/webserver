# pragma once

# include <map>
# include <string>
# include <vector>
# include "Defines.hpp"

class	RequestHeaders
{
	private:
		std::string		_raw;
		bool			_hasHost;
		std::string		_hostName;
		unsigned int	_hostPort;
		HttpStatusCode	_statusCode;

		std::map<std::string, std::string>					_headers;
		std::map<std::string, std::vector<std::string> >	_multiHeaders;

		std::string		_trimWhitespace(const std::string&);
		bool			_parseHostHeader(const std::string&);
		bool			_isValidHeaderName(const std::string&);
		std::string		_toLowercase(const std::string&) const;
		bool			_isValidHeaderValue(const std::string&);
		bool			_isDuplicated(std::string&, std::string&);

	public:
		RequestHeaders(const std::string&);

		void				clear();
		bool				parse();

		const std::string&	getHostName() const;
		unsigned int		getHostPort() const;
		HttpStatusCode		getStatusCode() const;
		bool				setState(bool, HttpStatusCode);

		bool				checkAndStore(std::string&, size_t);
		bool				hasHeader(const std::string&) const;

		const std::map<std::string, std::string>&	getHeadersMap() const;
		const std::vector<std::string>&				getMultiHeader(const std::string&) const;
		std::string									getHeaderValue(const std::string&) const;

};
