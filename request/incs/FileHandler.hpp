# pragma once

# include <string>
# include <cstddef>
# include <fcntl.h>
# include <unistd.h>
# include <sys/stat.h>
# include "Defines.hpp"

class	FileHandler
{
	private:
		int				_fd;
		std::string		_path;
		size_t			_size;
		size_t			_offset;
		bool			_isOpen;
		bool			_isTemp;
		std::string		_uploadDir;
		bool			_createTempBody(bool);
		bool			_createTempRequest(bool);
		bool			_createTempResponse(bool);
		bool			_createUploadFile(const std::string&);

	public:
		FileHandler();
		~FileHandler();
		static const std::string TEMP_DIR;

		void				close();
		bool				remove();
		bool				seek(size_t);
		ssize_t				read(char*, size_t);
		ssize_t				write(const char*, size_t );
		bool				create(FileType, bool, const std::string& = "");
		bool				open(const std::string& , int = O_RDWR | O_CREAT, mode_t = 0644);

		int					fd() const;
		size_t				size() const;
		const std::string&	path() const;
		bool				isOpen() const;
		bool				isTemp() const;
		size_t				offset() const;
		std::string			getUploadPath(const std::string&);

		static bool			exists(const std::string&);
		static bool			isDirectory(const std::string&);
		static bool			createDirectory(const std::string&);
};
