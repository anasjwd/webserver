# pragma once

# include <vector>
# include <string>
# include "Defines.hpp"
# include "FileHandler.hpp"

class	RequestBody
{
	private:
		bool						_expected;
		std::string					_boundary;
		bool						_isChunked;
		HttpStatusCode				_statusCode;
		FileHandler					_fileHandler;
		bool						_isMultipart;
		bool						_isCompleted;
		std::string					_contentType;
		size_t						_contentLength;
		size_t						_bytesReceived;

		bool						_inPart;
		// size_t						_partContentStart;
		FileHandler					_uploadHandler;
		std::vector<FileHandler>	_uploadHandlers;
		std::string					_currentFilename;
		std::string					_multipartBuffer;
		bool						_isCurrentPartFile;

		size_t						_chunkParsePos;
		size_t						_currentChunkSize;
		size_t						_bytesReceivedInChunk;


		bool						_parseChunkSize(const std::string&);
		bool						_processChunkData(const char*, size_t);
		bool						_processMultipartChunk(const char*, size_t);

	public:
		RequestBody();
		~RequestBody();

		void						clear();

		bool						isChunked() const;
		bool						isExpected() const;
		bool						isMultipart() const;
		bool						isCompleted() const;

		void						setExpected();
		void						setCompleted();
		void						setChunked(bool);
		void						setMultipart(bool);
		void						setContentLength(size_t);
		bool						setState(bool, HttpStatusCode);
		void						setContentType(const std::string&);

		const FileHandler   		getTempFile() const;
		const FileHandler&			getUploadHandler() const;
		HttpStatusCode				getStatusCode() const;
		std::vector<FileHandler>	getUploadedFiles() const;
		size_t						getContentLength() const;
		size_t						getBytesReceived() const;

		bool						receiveData(const char*, size_t);
		bool						extractBoundary(const std::string&);

};