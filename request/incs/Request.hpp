# pragma once

# include <string>
# include "Defines.hpp"
# include "RequestBody.hpp"
# include "RequestLine.hpp"
# include "RequestHeaders.hpp"

#define RESET   "\033[0m"
#define BG_YELLOW   "\033[4m"

class	Http;
class	Connection;

class	Request
{
	private:
		int				_fd;
		RequestLine		_rl;
		RequestHeaders	_rh;
		RequestBody		_rb;
		RequestState	_state;
		HttpStatusCode	_statusCode;

		std::string		_buffer;
		bool			_requestDone;

		bool			_processBodyHeaders();
		bool			_processContentLength();
		bool			_processChunkedTransfer();
		bool			_processType(std::string);
		bool			_connectionChecks(Connection*);
		bool			_isChunkedTransferEncoding(const std::string&);
		bool			_validateMethodBodyCompatibility(Connection *);

	public:
		Request();
		Request(int);

		void					clear();
		bool					stateChecker() const;
		bool					isRequestDone() const;

		void					setFd(int);
		bool					setState(bool, HttpStatusCode);

		const int&				getFd() const;
		const RequestState&		getState() const;
		const HttpStatusCode&	getStatusCode() const;
		const RequestLine&		getRequestLine() const;
		const RequestBody&		getRequestBody() const;
		const RequestHeaders&	getRequestHeaders() const;

		void					treatUploadLocation(Connection*);
		bool					contentLength(const std::string&);

		bool					bodySection();
		bool					lineSection();
		bool					headerSection(Connection*, Http*);
		bool					appendToBuffer(Connection*, Http*, const char*, size_t);
};
