# pragma once

# include <map>
# include <string>
# include <fstream>
# include <iostream>
# include "Defines.hpp"

class	RequestBody
{
	private:
		BodyType			_bodyType;
		std::fstream		_tempFile;
		bool				_isParsed;
		bool				_isChunked;
		HttpStatusCode		_statusCode;
		bool				_isCompleted;
		std::string			_contentType;
		std::string			_tempFilename;
		size_t				_contentLength;
		size_t				_bytesReceived;

		size_t				_currentChunkSize;
		size_t				_bytesReceivedInChunk;

		std::string			_rawData;
		std::string			_boundary;
		std::map<std::string, std::string>	_urlEncodedData;

		bool				_skipCRLF(size_t&);
		bool				_parsePartHeaders(size_t&, std::map<std::string, std::string>&);
		bool				_processPartData(size_t&, const std::string&, const std::map<std::string, std::string>&);
		bool				_processContentDisposition(const std::map<std::string, std::string>&, const std::string&);
		bool				_handleFileUpload(const std::string&, const std::string&, const std::string&);

		bool				_parseJson();
		bool				_parseMultipart();
		void				_cleanupTempFile();
		bool				_parseUrlEncoded();
		void				_determineBodyType();
		std::string			_extractBoundary(const std::string&);
		bool				_writeToTempFile(const char*, size_t);
		bool				_processChunkedData(const char*, size_t);

	public:
		RequestBody();
		~RequestBody();
		
		void				clear();
		bool				parse();

		bool				isParsed() const;
		bool				isChunked() const;
		bool				isCompleted() const;
		bool				receiveData(const char*, size_t);
		
		void				setCompleted();
		void				setChunked(bool);
		void				setContentLength(size_t);
		void				setContentType(const std::string&);
		bool				setState(bool, HttpStatusCode);

		const std::string&	getRawData() const;
		BodyType			getBodyType() const;
		HttpStatusCode		getStatusCode() const;
		const std::string&	getTempFilename() const;
		size_t				getContentLength() const;
		size_t				getBytesReceived() const;
		const std::map<std::string, std::string>&	getUrlEncodedData() const;

        bool				parseBodyContent();
		void				setHeadersContext(const std::string&, const std::string&);
};
