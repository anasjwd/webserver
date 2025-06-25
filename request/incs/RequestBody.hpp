# pragma once

# include <map>
# include <string>
# include <fstream>
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

		size_t				_chunkParsePos;
		size_t				_currentChunkSize;
		size_t				_bytesReceivedInChunk;

		std::string			_rawData;
		std::string			_boundary;
		std::map<std::string, std::string>	_urlEncodedData;

		bool				_parseChunkSize();
		bool				_processChunkData();

		std::string			_extractBoundary(const std::string&);

		void				_cleanupTempFile();
		void				_reWriteTempFile();
		void				_readTempFileData();
		bool				_validateMultipartBoundaries();
		bool				_writeToTempFile(const char*, size_t);

	public:
		RequestBody();
		~RequestBody();
		
		void				clear();

		bool				isParsed() const;
		bool				isChunked() const;
		bool				isCompleted() const;
		bool				receiveData(const char*, size_t);
		
		void				setCompleted();
		void				setChunked(bool);
		void				setContentLength(size_t);
		bool				setState(bool, HttpStatusCode);
		void				setContentType(const std::string&);

		const std::string&	getRawData() const;
		BodyType			getBodyType() const;
		HttpStatusCode		getStatusCode() const;
		const std::string&	getTempFilename() const;
		size_t				getContentLength() const;
		size_t				getBytesReceived() const;

};
