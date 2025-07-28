# include <cstdlib>
# include <sstream>
# include <iostream>
# include <unistd.h>
# include <sys/stat.h>
# include "../incs/RequestBody.hpp"

RequestBody::RequestBody()
	:	_expected(false), _isChunked(false), _statusCode(OK),
		_isMultipart(false), _isCompleted(false), _contentLength(0),
		_bytesReceived(0), _chunkParsePos(0), _currentChunkSize(0),
		_bytesReceivedInChunk(0)
{
	_fileHandler.create(TEMP_REQ);
}

RequestBody::~RequestBody()
{
	if (_uploadHandler.fd() != -1)
		std::cout << "Uploaded file path: " << _uploadHandler.path() << "\n";
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

		if (!_fileHandler.write(buffer.data() + _chunkParsePos, toWrite))
			return setState(false, INTERNAL_SERVER_ERROR);

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

bool RequestBody::_processMultipartChunk(const char* data, size_t len)
{
	std::cout << "\n[Multipart] Processing " << len << " bytes\n";

	_multipartBuffer.append(data, len);

	if (!_isCurrentPartFile)
	{
		if (!_fileHandler.write(data, len))
		{
			std::cerr << "[Multipart] Failed to write to _fileHandler\n";
			return setState(false, INTERNAL_SERVER_ERROR);
		}
		_bytesReceived += len;
	}

	if (!_inPart)
	{
		size_t dispPos = _multipartBuffer.find("Content-Disposition: form-data;");
		if (dispPos == std::string::npos)
			return true;

		size_t filenamePos = _multipartBuffer.find("filename=\"", dispPos);
		if (filenamePos != std::string::npos)
		{
			size_t start = filenamePos + 10;
			size_t end = _multipartBuffer.find('"', start);
			if (end != std::string::npos)
			{
				_currentFilename = _multipartBuffer.substr(start, end - start);
				_isCurrentPartFile = true;
				std::cout << "[Multipart] Found file part: " << _currentFilename << "\n";

				if (!_uploadHandler.create(UPLOAD_FILE, _currentFilename))
				{
					std::cerr << "[Multipart] Failed to create upload file\n";
					return setState(false, INTERNAL_SERVER_ERROR);
				}
				std::cout << "fd upload: " << _uploadHandler.fd() << ", path: " << _uploadHandler.path() << "\n";
			}
		}
		_inPart = true;
	}

	std::string boundaryLine = "--" + _boundary;
	size_t boundaryPos;
	while ((boundaryPos = _multipartBuffer.find(boundaryLine)) != std::string::npos)
	{
		std::cout << "[Multipart] Found boundary\n";

		size_t partDataStart = _multipartBuffer.find(CRLFCRLF);
		if (partDataStart == std::string::npos)
			break;

		partDataStart += 4;
		size_t partDataEnd = boundaryPos - 2;
		if (_isCurrentPartFile)
		{
			size_t partLen = partDataEnd - partDataStart;
			if (!_uploadHandler.write(_multipartBuffer.data() + partDataStart, partLen))
			{
				std::cerr << "[Multipart] Failed to write to upload file\n";
				return setState(false, INTERNAL_SERVER_ERROR);
			}
			std::cout << "[Multipart] Wrote " << partLen << " bytes to upload file\n";
		}

		// Advance past this boundary
		_multipartBuffer.erase(0, boundaryPos + boundaryLine.size());
		_isCurrentPartFile = false;
		_inPart = false;

		if (_multipartBuffer.find("--") == 0)
		{
			std::cout << "[Multipart] Final boundary found\n";
			_isCompleted = true;
			return true;
		}
	}

	return true;
}



void	RequestBody::clear()
{
	_statusCode = OK;

	_expected = false;
	_isChunked = false;
	_isMultipart = false;
	_isCompleted = false;

	_boundary.clear();
	_contentType.clear();

	_contentLength = 0;
	_bytesReceived = 0;
	_chunkParsePos = 0;
	_currentChunkSize = 0;
	_bytesReceivedInChunk = 0;
}

bool	RequestBody::isChunked() const
{
	return _isChunked;
}

bool	RequestBody::isExpected() const
{
	return _expected;
}

bool	RequestBody::isMultipart() const
{
	return _isMultipart;
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

void	RequestBody::setMultipart(bool isMultipart)
{
	_isMultipart = isMultipart;
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

const FileHandler	RequestBody::getTempFile() const
{
	return _fileHandler;
}

HttpStatusCode	RequestBody::getStatusCode() const
{
	return _statusCode;
}

std::vector<FileHandler>	RequestBody::getUploadedFiles() const
{
	return _uploadHandlers;
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

	std::cout << "\n\nChunked: " << _isChunked << ", multipart: " << _isMultipart << "\n\n";
	if (_isMultipart)
		return _processMultipartChunk(data, len);
	if (_isChunked) // chunked | contnt-length
		return _processChunkData(data, len);

	if (_bytesReceived + len > _contentLength)
		return setState(false, PAYLOAD_TOO_LARGE);

	if (!_fileHandler.write(data, len))
		return setState(false, INTERNAL_SERVER_ERROR);

	_bytesReceived += len;
	if (_bytesReceived >= _contentLength)
		_isCompleted = true;

	return true;
}
