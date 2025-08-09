# pragma once

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
		bool						_isCompleted;
		std::string					_contentType;
		size_t						_contentLength;
		size_t						_bytesReceived;
		size_t						_chunkParsePos;
		size_t						_currentChunkSize;
		size_t						_bytesReceivedInChunk;

		bool						_parseChunkSize(const std::string&);
		bool						_processChunkData(const char*, size_t);

	public:
		RequestBody();
		~RequestBody();

		void						clear();
		bool						create(FileType, std::string);

		bool						isChunked() const;
		bool						isCompleted() const;

		void						setExpected();
		void						setCompleted();
		void						setChunked(bool);
		void						setContentLength(size_t);
		bool						setState(bool, HttpStatusCode);
		void						setContentType(const std::string&);

		const FileHandler&			getTempFile() const;
		HttpStatusCode				getStatusCode() const;
		size_t						getContentLength() const;
		size_t						getBytesReceived() const;

		bool						receiveData(const char*, size_t);
};
