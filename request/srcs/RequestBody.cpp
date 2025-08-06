# include <cstdlib>
# include <sstream>
# include <iostream>
# include <unistd.h>
# include <sys/stat.h>
# include "../incs/RequestBody.hpp"

RequestBody::RequestBody()
	:	_expected(false), _isChunked(false), _statusCode(OK),
		_isCompleted(false), _contentLength(0), _bytesReceived(0),
		_chunkParsePos(0), _currentChunkSize(0), _bytesReceivedInChunk(0)
{
}

RequestBody::~RequestBody()
{
	if (_fileHandler.fd() != -1)
		std::cout << "Body file path: " << _fileHandler.path() << "\n";
}

bool	RequestBody::_parseChunkSize(const std::string& buf)
{
	size_t pos = buf.find(CRLF, _chunkParsePos);
	if (pos == std::string::npos)
		return false;

	std::string chunkSizeStr = buf.substr(_chunkParsePos, pos - _chunkParsePos);
	std::istringstream iss(chunkSizeStr);
	iss >> std::hex >> _currentChunkSize;
	chunkSizeStr.clear();

	if (iss.fail())
		return setState(false, BAD_REQUEST);

	_chunkParsePos = pos + 2;
	iss.clear();
	return true;
}

bool	RequestBody::_processChunkData(const char* data, size_t len)
{
	std::string buffer(data, len);
	_chunkParsePos = 0;

	while (_chunkParsePos < buffer.size())
	{
		if (_currentChunkSize == 0)
		{
			if (!_parseChunkSize(buffer))
				return false;
			if (_currentChunkSize == 0)
			{
				_isCompleted = true;
				break;
			}
		}

		size_t available = buffer.size() - _chunkParsePos;
		size_t toWrite = std::min(_currentChunkSize - _bytesReceivedInChunk, available);

		if (_fileHandler.write(buffer.data() + _chunkParsePos, toWrite) == -1)
		{
			std::cout << "write:reqbody.cpp 4 \n";	
			return setState(false, INTERNAL_SERVER_ERROR);
		}

		_chunkParsePos += toWrite;
		_bytesReceivedInChunk += toWrite;
		_bytesReceived += toWrite;

		if (_bytesReceivedInChunk == _currentChunkSize)
		{
			_bytesReceivedInChunk = 0;
			_currentChunkSize = 0;
			if (buffer.substr(_chunkParsePos, 2) == CRLF)
				_chunkParsePos += 2;
			else
				return false;
		}
	}

	return true;
}

void	RequestBody::clear()
{
	_statusCode = OK;

	_expected = false;
	_isChunked = false;
	_isCompleted = false;

	_boundary.clear();
	_contentType.clear();

	_contentLength = 0;
	_bytesReceived = 0;
	_chunkParsePos = 0;
	_currentChunkSize = 0;
	_bytesReceivedInChunk = 0;
}

bool	RequestBody::create(FileType type)
{
	if (_fileHandler.fd() != -1)
	{
		std::cerr << "Temporary file already exists, cannot create a new one\n";
		return setState(false, INTERNAL_SERVER_ERROR);
	}

	if (!_fileHandler.create(type))
	{
		std::cerr << "Failed to create temporary file for request body\n";
		return setState(false, INTERNAL_SERVER_ERROR);
	}

	std::cout << "Temporary file created at: " << _fileHandler.path() << "\n";
	return true;
}

bool	RequestBody::isChunked() const
{
	return _isChunked;
}

bool	RequestBody::isExpected() const
{
	return _expected;
}

bool	RequestBody::isCompleted() const
{
	return _isCompleted;
}


void	RequestBody::setExpected()
{
	_expected = true;
}

void	RequestBody::setCompleted()
{
	_isCompleted = true;
}

void	RequestBody::setChunked(bool isChunked)
{
	_isChunked = isChunked;
}

void	RequestBody::setContentLength(size_t length)
{
	_contentLength = length;
}

bool	RequestBody::setState(bool tof, HttpStatusCode status)
{
	_statusCode = status;
	return tof;
}

void	RequestBody::setContentType(const std::string& contentType)
{
	_contentType = contentType;
}

const FileHandler&	RequestBody::getTempFile() const
{
	return _fileHandler;
}

HttpStatusCode	RequestBody::getStatusCode() const
{
	return _statusCode;
}

size_t	RequestBody::getContentLength() const
{
	return _contentLength;
}

size_t	RequestBody::getBytesReceived() const
{
	return _bytesReceived;
}

bool	RequestBody::extractBoundary(const std::string& contentType)
{
	size_t pos = contentType.find("boundary=");
	if (pos == std::string::npos)
		return setState(false, BAD_REQUEST);
	_boundary = contentType.substr(pos + 9);
	return true;
}

bool	RequestBody::receiveData(const char* data, size_t len)
{
	if (!_expected || _isCompleted || !data || len == 0)
		return false;

	if (_isChunked)
		return _processChunkData(data, len);

	if (_fileHandler.write(data, len) == -1)
	{
        std::cout << "write:reqbody.cpp 1 \n";	
		return setState(false, INTERNAL_SERVER_ERROR);
	}

	_bytesReceived += len;
	if (_bytesReceived >= _contentLength)
		_isCompleted = true;

	return true;
}
