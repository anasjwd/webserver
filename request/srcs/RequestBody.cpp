# include <cstdlib>
# include <sstream>
# include <unistd.h>
# include "../incs/RequestBody.hpp"

RequestBody::RequestBody()
	:	_isParsed(false), _isChunked(false), _statusCode(START),
		_isMultipart(false), _isCompleted(false),
		_contentLength(0), _bytesReceived(0), _chunkParsePos(0),
		_currentChunkSize(0), _bytesReceivedInChunk(0)
{
	char temp[] = "/tmp/webserv_body_XXXXXX";
	int fd = mkstemp(temp);
	if (fd == -1)
		setState(false, INTERNAL_SERVER_ERROR);
	else
	{
		_tempFilename = temp;
		close(fd);
	}
}

RequestBody::~RequestBody()
{
	_cleanupTempFile();
}

bool	RequestBody::_parseChunkSize()
{
	_readTempFileData();
	
	size_t crlf_pos = _rawData.find(CRLF, _chunkParsePos);
	if (crlf_pos == std::string::npos)
		return false;

	std::string hexSize = _rawData.substr(_chunkParsePos, crlf_pos - _chunkParsePos);
	
	size_t semicolon_pos = hexSize.find(';');
	if (semicolon_pos != std::string::npos)
		hexSize = hexSize.substr(0, semicolon_pos);

	char* endptr;
	_currentChunkSize = strtoul(hexSize.c_str(), &endptr, 16);
	
	if (*endptr != '\0' || hexSize.empty())
		return setState(false, BAD_REQUEST);

	_chunkParsePos = crlf_pos + 2;
	return true;
}

bool	RequestBody::_processChunkData()
{
	if (_currentChunkSize == 0)
	{
		_isCompleted = true;
		_chunkParsePos = _rawData.size();
		return true;
	}

	size_t data_end = _chunkParsePos + _currentChunkSize;
	if (data_end + 2 > _rawData.size())
		return false;

	if (_rawData.substr(data_end, 2) != CRLF)
		return setState(false, BAD_REQUEST);

	_chunkParsePos = data_end + 2;
	_currentChunkSize = 0;
	return true;
}

void	RequestBody::_cleanupTempFile()
{
	if (!_tempFilename.empty())
	{
		if (_tempFile.is_open())
			_tempFile.close();
		remove(_tempFilename.c_str());
		_tempFilename.clear();
	}
}

void	RequestBody::_reWriteTempFile()
{
	if (_tempFile.is_open())
		_tempFile.close();

	std::ofstream file(_tempFilename.c_str(), std::ios::binary | std::ios::trunc);
	if (file.is_open())
	{
		file.write(_rawData.data(), _rawData.size());
		file.close();
	}
}

void	RequestBody::_readTempFileData()
{
	if (_tempFile.is_open())
		_tempFile.close();

	std::ifstream file(_tempFilename.c_str(), std::ios::binary);
	if (file.is_open())
	{
		std::ostringstream oss;
		oss << file.rdbuf();
		_rawData = oss.str();
		file.close();
	}
}

bool	RequestBody::_writeToTempFile(const char* data, size_t length)
{
	if (!_tempFile.is_open())
	{
		_tempFile.open(_tempFilename.c_str(), std::ios::binary | std::ios::app);
		if (!_tempFile.is_open())
			return setState(false, INTERNAL_SERVER_ERROR);
	}

	_tempFile.write(data, length);
	_tempFile.flush();
	if (_tempFile.fail())
		return setState(false, INTERNAL_SERVER_ERROR);

	_bytesReceived += length;

	if (!_isChunked && _contentLength > 0 && _bytesReceived >= _contentLength)
	{
		setCompleted();
		_tempFile.close();
	}

	return true;
}

bool	RequestBody::_validateMultipartBoundaries() 
{
	if (_boundary.empty())
		return setState(false, BAD_REQUEST);

	_readTempFileData();

	std::string startBoundary = "--" + _boundary + CRLF;
	std::string endBoundary = "--" + _boundary + "--" + CRLF;

	size_t startPos = _rawData.find(startBoundary);
	if (startPos != 0)
		return setState(false, BAD_REQUEST);

	size_t endPos = _rawData.find(endBoundary);
	if (endPos == std::string::npos)
		return setState(false, BAD_REQUEST);

	if (endPos + endBoundary.length() < _rawData.length()) {
		std::string remaining = _rawData.substr(endPos + endBoundary.length());
		if (!remaining.empty() && remaining != CRLF)
			return setState(false, BAD_REQUEST);
	}

	return true;
}

void	RequestBody::clear()
{
	_cleanupTempFile();
	
	char temp[] = "/tmp/webserv_body_XXXXXX";
	int fd = mkstemp(temp);
	if (fd == -1)
		setState(false, INTERNAL_SERVER_ERROR);
	else
	{
		_tempFilename = temp;
		close(fd);
	}

	_rawData.clear();
	_boundary.clear();
	_isParsed = false;
	_isChunked = false;
	_contentLength = 0;
	_bytesReceived = 0;
	_chunkParsePos = 0;
	_isCompleted = false;
	_currentChunkSize = 0;
	_bytesReceivedInChunk = 0;
}

bool	RequestBody::isParsed() const
{
	return _isParsed;
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
# include <iostream>

bool RequestBody::_extractFileFromMultipart()
{
    if (!_isMultipart || _boundary.empty())
	{
		std::cout << "Multipart: " << isMultipart() << " | boundary: " << _boundary << std::endl;
        std::cerr << "Upload failed: Not multipart or missing boundary\n";
        return false;
    }

    _readTempFileData();
    
    std::string boundary = "--" + _boundary;
    size_t file_start = _rawData.find("filename=\"", _rawData.find(boundary));
    if (file_start == std::string::npos) {
        std::cerr << "Upload failed: No filename found in multipart data\n";
        return false;
    }
    
    file_start += 10;
    size_t file_end = _rawData.find("\"", file_start);
    std::string filename = _rawData.substr(file_start, file_end - file_start);

    size_t content_start = _rawData.find(END_HEADER, file_end) + 4;
    if (content_start == std::string::npos + 4) return false;
    
    size_t content_end = _rawData.find(boundary, content_start) - 2;
    if (content_end == std::string::npos - 2) return false;
    
    std::string file_content = _rawData.substr(content_start, content_end - content_start);

    std::string upload_path = "/tmp/uploads/" + filename;
    std::ofstream out(upload_path.c_str(), std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Upload failed: Could not create file " << upload_path << "\n";
        return false;
    }

    out.write(file_content.data(), file_content.size());
    out.close();
    
    std::cerr << "File uploaded successfully: " << upload_path 
              << " (" << file_content.size() << " bytes)\n";
    return true;
}

bool	RequestBody::receiveData(const char* data, size_t length)
{
	if (isCompleted())
		return false;

	if (!_writeToTempFile(data, length))
		return false;

	if (isMultipart() && isCompleted())
	{
		// if (!_validateMultipartBoundaries())
		// {
		// 	std::cerr << "Invalid multipart boundaries\n";
		// 	return false;
		// }
		if (!_extractFileFromMultipart())
		{
			std::cerr << "Failed to extract file from multipart data\n";
			return false;
		}
		setCompleted();
		return true;
	}
	else if (isChunked())
	{
		_readTempFileData();
		if (!_processChunkData())
			return false;
		_reWriteTempFile();
	}
	else if (_contentLength > 0 && _bytesReceived >= _contentLength)
		setCompleted();

	return true;
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

void	RequestBody::setMultipart(bool tof)
{
	_isMultipart = tof;
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

const std::string&	RequestBody::getRawData() const
{
	return _rawData;
}

HttpStatusCode	RequestBody::getStatusCode() const
{
	return _statusCode;
}

const std::string&	RequestBody::getTempFilename() const
{
	return _tempFilename;
}

size_t	RequestBody::getContentLength() const
{
	return _contentLength;
}

size_t	RequestBody::getBytesReceived() const
{
	return _bytesReceived;
}

std::string	RequestBody::extractBoundary(const std::string& contentType)
{
	std::cout << "Extracting boundary!\n";
	size_t boundaryPos = contentType.find("boundary=");
	if (boundaryPos == std::string::npos)
		return "";

	boundaryPos += 9;
	std::string boundary = contentType.substr(boundaryPos);
	
	// Trim whitespace and quotes
	size_t start = boundary.find_first_not_of(" \t\"");
	if (start == std::string::npos)
		return "";
	
	size_t end = boundary.find_last_not_of(" \t\"");
	_boundary = boundary.substr(start, end - start + 1);
	return _boundary;
}