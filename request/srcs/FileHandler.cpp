# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <iostream>
# include <unistd.h>
# include <stdexcept>
# include "../incs/FileHandler.hpp"

FileHandler::FileHandler() 
	:	_fd(-1), _size(0), _offset(0),
		_isOpen(false), _isTemp(false)
{
}

FileHandler::~FileHandler()
{
	close();
	// if (_isPost == false)
	// 	remove();
}

bool	FileHandler::_createTempBody()
{
	char path[] = "/tmp/webserv_reqBody_XXXXXX";

	_fd = mkstemp(path);
	if (_fd == -1)
		return false;

	_size = 0;
	_offset = 0;
	_path = path;
	_isTemp = true;
	_isOpen = true;
	return true;
}

bool	FileHandler::_createTempRequest()
{
	char path[] = "/tmp/webserv_request_XXXXXX";

	_fd = mkstemp(path);
	if (_fd == -1)
		return false;

	_size = 0;
	_offset = 0;
	_path = path;
	_isTemp = true;
	_isOpen = true;
	return true;
}

bool	FileHandler::_createTempResponse()
{
	char path[] = "/tmp/webserv_response_XXXXXX";

	_fd = mkstemp(path);
	if (_fd == -1)
		return false;

	_size = 0;
	_offset = 0;
	_path = path;
	_isTemp = true;
	_isOpen = true;
	return true;
}

bool	FileHandler::_createUploadFile(const std::string& filename)
{
	std::cout << "Creating upload file named: " << filename << "\n"; 
	if (filename.empty())
		throw std::runtime_error("Filename required for upload file");

	char buf[WS_PATH_MAX];

	if (getcwd(buf, WS_PATH_MAX))
		_uploadDir = std::string(buf) + "/upload";

	if (!exists(_uploadDir) && !createDirectory(_uploadDir))
		throw std::runtime_error("Failed to create upload directory");

	std::string full_path = getUploadPath(filename);
	_fd = ::open(full_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (_fd == -1)
		return false;

	_size = 0;
	_offset = 0;
	_isOpen = true;
	_isTemp = false;
	_path = full_path;
	std::cout << "Succesfully created!\n"; 
	return true;
}

void	FileHandler::close()
{
	if (_isOpen)
	{
		::close(_fd);
		_fd = -1;
		_isOpen = false;
	}
}

bool	FileHandler::remove()
{
	if (_isOpen)
		close();

	if (!_path.empty())
	{
		if (std::remove(_path.c_str()))
			return false;

		_path.clear();
		_isTemp = false;
	}
	return true;
}

bool	FileHandler::seek(size_t offset)
{
	if (!_isOpen)
		return false;

	off_t result = lseek(_fd, offset, SEEK_SET);
	if (result == (off_t)-1)
		return false;

	_offset = offset;
	return true;
}

ssize_t	FileHandler::read(char* buffer, size_t size)
{
	if (!_isOpen)
		return -1;

	ssize_t bytesRead = ::read(_fd, buffer, size);
	if (bytesRead > 0)
		_offset += bytesRead;

	return bytesRead;
}	

ssize_t	FileHandler::write(const char* data, size_t size)
{
	if (!_isOpen)
		return -1;

	ssize_t written = ::write(_fd, data, size);
	if (written > 0)
	{
		_offset += written;
		if (_offset > _size)
			_size = _offset;
	}	
	return written;
}

bool	FileHandler::create(FileType type, const std::string& filename)
{
	if (_isOpen)
		close();

	try
	{
		switch (type)
		{
			case TEMP_BODY:
				return _createTempBody();
			case TEMP_REQ:
				return _createTempRequest();
			case UPLOAD_FILE:
				return _createUploadFile(filename);
			default:
				throw std::runtime_error("Invalid file type");
		}
	}
	catch (const std::exception& e)
	{
		if (_fd != -1)
		{
			::close(_fd);
			_fd = -1;
		}
		return false;
	}
}

bool	FileHandler::open(const std::string& path, int flags, mode_t mode)
{
	if (_isOpen)
		close();

	_fd = ::open(path.c_str(), flags, mode);
	if (_fd == -1)
		return false;

	_offset = 0;
	_path = path;
	_isOpen = true;
	_isTemp = false;

	struct stat st;
	if (fstat(_fd, &st))
	{
		::close(_fd);
		_fd = -1;
		return false;
	}
	_size = st.st_size;

	return true;
}	

int	FileHandler::fd() const
{
	return _fd;
}

size_t	FileHandler::size() const
{
	return _size;
}

const std::string&	FileHandler::path() const
{
	return _path;
}

bool	FileHandler::isOpen() const
{
	return _isOpen;
}

bool	FileHandler::isTemp() const
{
	return _isTemp;
}

size_t	FileHandler::offset() const
{
	return _offset;
}

std::string	FileHandler::getUploadPath(const std::string& filename)
{
	return _uploadDir + "/" + filename;
}

bool	FileHandler::exists(const std::string& path)
{
	struct stat st;
	return (stat(path.c_str(), &st) == 0);
}

bool	FileHandler::isDirectory(const std::string& path)
{
	struct stat st;
	if (stat(path.c_str(), &st))
		return false;

	return S_ISDIR(st.st_mode);
}

bool	FileHandler::createDirectory(const std::string& path)
{
	return (mkdir(path.c_str(), 0755) == 0);
}
