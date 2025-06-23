# include <cstdlib>
# include <unistd.h>
# include <sstream>
# include "../incs/RequestBody.hpp"

RequestBody::RequestBody()
	:	_bodyType(RAW), _isParsed(false), _isChunked(false),
		_isCompleted(false), _contentLength(0), _bytesReceived(0), 
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

bool    RequestBody::_parseJson()
{
	// Will be implemented after being sure that urlEncoded and multipart works fine.
	return true;
}

bool	RequestBody::_skipCRLF(size_t& pos)
{
	if (pos >= _rawData.size())
		return false;
	
	if (_rawData[pos] == '\r')
	{
		pos++;
		if (pos >= _rawData.size() || _rawData[pos] != '\n')
			return false;
	}
	pos++;
	return true;
}

bool	RequestBody::_parsePartHeaders(size_t& pos, std::map<std::string, std::string>& headers) 
{
	while (pos < _rawData.size())
	{
		if (_rawData[pos] == '\r' || _rawData[pos] == '\n')
		{
			if (!_skipCRLF(pos))
				return false;
			break;
		}

		size_t colonPos = _rawData.find(':', pos);
		if (colonPos == std::string::npos) {
		return setState(false, BAD_REQUEST);
		}

		std::string name = _rawData.substr(pos, colonPos - pos);
		pos = colonPos + 1;

		while (pos < _rawData.size() && isspace(_rawData[pos]))
			pos++;

		size_t endPos = _rawData.find(CRLF, pos);
		if (endPos == std::string::npos)
			endPos = _rawData.size();

		std::string value = _rawData.substr(pos, endPos - pos);
		headers[name] = value;
		pos = endPos;

		if (!_skipCRLF(pos))
			return false;
	}

	return true;
}

bool	RequestBody::_processPartData(size_t& pos, const std::string& boundary, const std::map<std::string, std::string>& headers)
{
	size_t nextBoundaryPos = _rawData.find(boundary, pos);
	if (nextBoundaryPos == std::string::npos)
		return setState(false, BAD_REQUEST);

	std::string partData = _rawData.substr(pos, nextBoundaryPos - pos);
	while (!partData.empty() && (partData[partData.length()-1] == '\r' || partData[partData.length()-1] == '\n'))
		partData.erase(partData.length()-1, 1);

	if (!_processContentDisposition(headers, partData))
		return false;

	pos = nextBoundaryPos + boundary.length();

	if (pos + 2 <= _rawData.size() && _rawData.substr(pos, 2) == "--")
		pos = _rawData.size(); // Skip to end

	return true;
}

bool RequestBody::_handleFileUpload(const std::string& fieldName, const std::string& filename, const std::string& data)
{
	char tempPath[] = "/tmp/webserv_upload_XXXXXX";
	int fd = mkstemp(tempPath);
	if (fd == -1)
		return setState(false, INTERNAL_SERVER_ERROR);

	close(fd);

	std::ofstream outFile(tempPath, std::ios::binary);
	if (!outFile)
		return setState(false, INTERNAL_SERVER_ERROR);

	outFile.write(data.data(), data.size());
	outFile.close();

	std::stringstream sizeStream;
	sizeStream << data.size();

	_urlEncodedData[fieldName + "_filename"] = filename;
	_urlEncodedData[fieldName + "_filepath"] = tempPath;
	_urlEncodedData[fieldName + "_filesize"] = sizeStream.str();

	return true;
}

bool	RequestBody::_processContentDisposition(const std::map<std::string, std::string>& headers, const std::string& data)
{
	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Disposition");
	if (it == headers.end())
		return setState(false, BAD_REQUEST);

	std::string name;
	std::string filename;
	std::string contentDisp = it->second;

	size_t namePos = contentDisp.find("name=\"");
	if (namePos == std::string::npos)
		return setState(false, BAD_REQUEST);

	namePos += 6;
	size_t nameEnd = contentDisp.find("\"", namePos);
	if (nameEnd == std::string::npos)
		return setState(false, BAD_REQUEST);

	name = contentDisp.substr(namePos, nameEnd - namePos);

	size_t filePos = contentDisp.find("filename=\"");
	if (filePos != std::string::npos)
	{
		filePos += 10;
		size_t fileEnd = contentDisp.find("\"", filePos);
		if (fileEnd == std::string::npos)
			return setState(false, BAD_REQUEST);
		filename = contentDisp.substr(filePos, fileEnd - filePos);
	}

	if (!filename.empty())
		return _handleFileUpload(name, filename, data);
	else
		_urlEncodedData[name] = data;

	return true;
}

bool	RequestBody::_parseMultipart()
{
	// Will be implemented after being sure that urlEncoded works fine.
	if (_boundary.empty())
		return setState(false, BAD_REQUEST);

	std::string boundary = "--" + _boundary;
	size_t boundaryLength = boundary.length();
	size_t pos = 0;

	if (_rawData.substr(0, boundaryLength) != boundary)
		return setState(false, BAD_REQUEST);

	pos = boundaryLength;

	while (pos < _rawData.size())
	{
		if (!_skipCRLF(pos))
			return setState(false, BAD_REQUEST);

		std::map<std::string, std::string> partHeaders;
		if (!_parsePartHeaders(pos, partHeaders))
			return false;

		if (!_processPartData(pos, boundary, partHeaders))
			return false;
	}

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

static std::string	urlDecode(const std::string& str)
{
	std::string	result;

	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] == '%' && i + 2 < str.size())
		{
			if (!isxdigit(str[i+1]) || !isxdigit(str[i+2]))
				return "";
			char hex[3] = {str[i+1], str[i+2], '\0'};
			char *end;
			unsigned long val = strtoul(hex, &end, 16);
			if (*end != '\0' || val > 255)
				return "";
			result += static_cast<char>(val);
			i += 2;
		} 
		else if (str[i] == '+')
			result += ' ';
		else
			result += str[i];
	}
	return result;
}

bool    RequestBody::_parseUrlEncoded()
{
	if (_rawData.empty())
		return true;

	size_t pos = 0;
	size_t pairCount = 0;
	_urlEncodedData.clear();
	while (pos < _rawData.size() && pairCount < MAX_HEADER)
	{
		size_t equalPos = _rawData.find('=', pos);
		if (equalPos == std::string::npos)
		{
			std::string key = urlDecode(_rawData.substr(pos));
			if (key.empty())
				return setState(false, BAD_REQUEST);
			_urlEncodedData[key] = "";
			break;
		}

		std::string key = _rawData.substr(pos, equalPos - pos);
		pos = equalPos + 1;

		size_t amPos = _rawData.find('&', pos);
		std::string value = _rawData.substr(pos, amPos - pos);
		pos = amPos == std::string::npos ? _rawData.size() : amPos + 1;

		key = urlDecode(key);
		value = urlDecode(value);
		_urlEncodedData[key] = value;
		pairCount++;

		if (amPos == std::string::npos)
			break;
	}
	return true;
}

void	RequestBody::_determineBodyType()
{
	if (_contentType.empty())
	{
		_bodyType = RAW;
		return;
	}

	if (_contentType.find("application/x-www-form-urlencoded") != std::string::npos)
		_bodyType = URL_ENCODED;
	else if (_contentType.find("multipart/form-data") != std::string::npos){
		_bodyType = MULTIPART;
		_boundary = _extractBoundary(_contentType);
	}
	else if (_contentType.find("application/json") != std::string::npos)
		_bodyType = JSON;
	else
		_bodyType = RAW;
}

std::string	RequestBody::_extractBoundary(const std::string& contentType)
{
	size_t boundaryPos = contentType.find("boundary=");
	if (boundaryPos == std::string::npos)
		return "";

	boundaryPos += 9; // Skip "boundary="
	std::string boundary = contentType.substr(boundaryPos);
	
	// Trim whitespace and quotes
	size_t start = boundary.find_first_not_of(" \t\"");
	if (start == std::string::npos)
		return "";
	
	size_t end = boundary.find_last_not_of(" \t\"");
	return boundary.substr(start, end - start + 1);
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
	
	_bodyType = RAW;
	_rawData.clear();
	_boundary.clear();
	_isParsed = false;
	_isChunked = false;
	_contentLength = 0;
	_bytesReceived = 0;
	_isCompleted = false;
	_currentChunkSize = 0;
	_urlEncodedData.clear();
	_bytesReceivedInChunk = 0;
}

bool	RequestBody::parse()
{
	if (isParsed())
		return false;

	if (_tempFile.is_open())
		_tempFile.close();
	
	std::ifstream file(_tempFilename.c_str(), std::ios::binary);
	if (!file.is_open())
		return false;

	std::ostringstream oss;
	oss << file.rdbuf();
	_rawData = oss.str();
	file.close();

	_determineBodyType();

	switch (_bodyType)
	{
		case URL_ENCODED:
			_isParsed = _parseUrlEncoded();
			break;
		case MULTIPART:
			_isParsed = _parseMultipart();
			break;
		case JSON:
			_isParsed = _parseJson();
			break;
		case RAW:
			_isParsed = true;
			break;
	}

	if (isParsed())
		return setState(true, OK);
	return false;
}

bool	RequestBody::isParsed() const
{
	return _isParsed;
}

bool	RequestBody::isChunked() const
{
	return _isChunked;
}

bool	RequestBody::isCompleted() const
{
	return _isCompleted;
}

bool	RequestBody::receiveData(const char* data, size_t length)
{
	if (_isCompleted)
		return false;

	if (length == 0)
	{
		if (_isChunked)
			return true;
		std::cout << "Set completed in receiveData with length 0.\n";
		setCompleted();
		return true;
	}

	if (!_writeToTempFile(data, length))
		return false;

	if (!_isChunked && _contentLength > 0 && _bytesReceived >= _contentLength)
		setCompleted();

	return true;
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

void	RequestBody::setContentType(const std::string& contentType)
{
	_contentType = contentType;
}
bool	RequestBody::setState(bool tof, HttpStatusCode status)
{
	_statusCode = status;
	return tof;
}

const std::string&	RequestBody::getRawData() const
{
	return _rawData;
}

BodyType	RequestBody::getBodyType() const
{
	return _bodyType;
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

const std::map<std::string, std::string>&	RequestBody::getUrlEncodedData() const
{
	return _urlEncodedData;
}

bool    RequestBody::parseBodyContent()
{
	return true;
}

static bool	isChunkedTransferEncoding(const std::string& transferEncoding)
{
	if (transferEncoding.empty())
		return false;

	size_t chunkedPos = transferEncoding.rfind("chunked");
	if (chunkedPos == std::string::npos)
		return false;

	std::string afterChunked = transferEncoding.substr(chunkedPos + 7);
	if (afterChunked.find_first_not_of(" ,\t\r\n") != std::string::npos)
		return false;

	return true;
}

void    RequestBody::setHeadersContext(const std::string& contentType, const std::string& transferEncoding)
{
	_contentType = contentType;
	_isChunked = isChunkedTransferEncoding(transferEncoding);
	_determineBodyType();
}
