# pragma once

# include <string>
# include "Defines.hpp"
# include "RequestBody.hpp"
# include "RequestLine.hpp"
# include "RequestHeaders.hpp"

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
		time_t			_lastActivityTime;

		bool			_processBodyHeaders();
		bool			_processContentLength();
		bool			_processChunkedTransfer();
		bool			_validateMethodBodyCompatibility();
		std::string		_extractBoundary(const std::string&);
		bool			_isChunkedTransferEncoding(const std::string&);

	public:
		Request();
		Request(int);

		void					clear();
		bool					stateChecker() const;
		bool					isRequestDone() const;

		void					setFd(int);
		const int&				getFd() const;
		bool					checkForTimeout() const;
		time_t					getLastActivityTime() const;
		void					setLastActivityTime(time_t time);

		const RequestState&		getState() const;
		const HttpStatusCode&	getStatusCode() const;
		const RequestLine&		getRequestLine() const;
		const RequestBody&		getRequestBody() const;
		const RequestHeaders&	getRequestHeaders() const;
		bool					setState(bool, HttpStatusCode);

		bool					isMultipart(const std::string&);
		bool					contentLength(const std::string&);

		bool					bodySection();
		bool					lineSection();
		bool					headerSection();
		bool					appendToBuffer(const char*, size_t);
};
