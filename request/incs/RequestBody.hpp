# pragma once

# include <string>
# include <fstream>
# include "Defines.hpp"

class	RequestBody
{
	private:
		std::string			_rawData;
		bool				_expected;
		std::string			_boundary;
		std::fstream		_tempFile;
		bool				_isParsed;
		bool				_isChunked;
		HttpStatusCode		_statusCode;
		bool				_isMultipart;
		bool				_isCompleted;
		std::string			_contentType;
		std::string			_tempFilename;
		size_t				_contentLength;
		size_t				_bytesReceived;

		size_t				_chunkParsePos;
		size_t				_currentChunkSize;
		size_t				_bytesReceivedInChunk;


		bool				_parseChunkSize();
		bool				_processChunkData();

		void				_cleanupTempFile();
		void				_reWriteTempFile();
		void				_readTempFileData();
		bool				_extractFileFromMultipart();
		bool				_validateMultipartBoundaries();
		bool				_writeToTempFile(const char*, size_t);

	public:
		RequestBody();
		~RequestBody();
		
		void				clear();

		bool				isParsed() const;
		bool				isChunked() const;
		bool				isExpected() const;
		bool				isMultipart() const;
		bool				isCompleted() const;
		bool				receiveData(const char*, size_t);
		
		void				setExpected();
		void				setCompleted();
		void				setChunked(bool);
		void				setMultipart(bool);
		void				setContentLength(size_t);
		bool				setState(bool, HttpStatusCode);
		void				setContentType(const std::string&);

		const std::string&	getRawData() const;
		HttpStatusCode		getStatusCode() const;
		const std::string&	getTempFilename() const;
		size_t				getContentLength() const;
		size_t				getBytesReceived() const;

		std::string			extractBoundary(const std::string&);

};