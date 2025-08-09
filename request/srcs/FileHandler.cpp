# include <cstdio>
# include <cstdlib>
# include <climits>
# include <cstring>
# include <iostream>
# include <unistd.h>
# include <stdexcept>
# include "../incs/FileHandler.hpp"
# include "../../response/include/Response.hpp"

FileHandler::FileHandler() 
	:	_fd(-1), _size(0), _offset(0),
		_isOpen(false), _isTemp(false)
{
}

FileHandler::~FileHandler()
{
	close();
	if (_isTemp)
	{
		std::cout << BBLUE << "Removing temporary file: " << _path << "\n" << RESET;
		remove();
	}
}

bool	FileHandler::_createBodyFile(bool isTemp, std::string uploadDir)
{
	char pathTemplate[PATH_MAX];

	std::cout << "Body file to be created, isTmp: " << isTemp << ", uploadDir: " << (uploadDir == "" ? "NULL": uploadDir) << "\n";

	if (uploadDir != "" && !exists(uploadDir) && !createDirectory(uploadDir))
	{
		std::cout << "Directory wasn't created!\n";
		return false;
	}
	if (uploadDir == "")
		std::cout << "upload dir is NULL\n";
	else
		std::cout << "Directory was created!\n";

	std::snprintf(pathTemplate, sizeof(pathTemplate), "%s/webserv_request_body_XXXXXX", uploadDir != "" ? uploadDir.c_str() : "/tmp");

	_fd = mkstemp(pathTemplate);
	if (_fd == -1)
	{
		std::cout << "Fail: mkstemp\n";
		return false;
	}
	
	_size = 0;
	_offset = 0;
	_isOpen = true;
	_isTemp = isTemp;
	_path = pathTemplate;
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
	{
		std::cout << "Filehandler::write fail: file not open!\n";
		return -1;
	}
	if (_fd == -1)
	{
		std::cout << "Filehandler::fd: -1!\n";
		return _fd;
	}

	ssize_t written = ::write(_fd, data, size);
	if (written > 0)
	{
		_offset += written;
		if (_offset > _size)
			_size = _offset;
	}	
	std::cout << "Filehandler::write size:" << written << "\n";
	return written;
}

bool	FileHandler::create(FileType type, std::string uploadDir)
{
	if (_isOpen)
		close();

	std::cout << "Creating file: " << type << ", expected 1;\n";
	if (uploadDir != "")
		std::cout << "UploadDir is: " << uploadDir << "\n";
	else
		std::cout << "UploadDir is: NULL\n";
	try
	{
		switch (type)
		{
			case POST_BODY:
				return _createBodyFile(false, uploadDir);
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
