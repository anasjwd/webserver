# pragma once

# include <string>
# include "Defines.hpp"
# include "RequestBody.hpp"
# include "RequestLine.hpp"
# include "RequestHeaders.hpp"

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
		bool			_connectionChecks(Http*, Connection*);
		bool			_isChunkedTransferEncoding(const std::string&);
		bool			_validateMethodBodyCompatibility(Http*, Connection *);


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

		bool					isMultipart(const std::string&);
		bool					contentLength(const std::string&);

		bool					bodySection();
		bool					lineSection();
		bool					headerSection(Connection*, Http*);
		bool					appendToBuffer(Connection*, Http*, const char*, size_t);
};
