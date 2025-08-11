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

		bool			_createBodyFile(bool, std::string = "");
		
	public:
		FileHandler();
		~FileHandler();

		void				close();
		bool				remove();
		bool				seek(size_t);
		ssize_t				read(char*, size_t);
		ssize_t				write(const char*, size_t );
		bool				create(FileType, std::string = "");
		bool				open(const std::string& , int = O_RDWR | O_CREAT, mode_t = 0644);

		int					fd() const;
		size_t				size() const;
		const std::string&	path() const;
		bool				isOpen() const;
		bool				isTemp() const;
		size_t				offset() const;

		static bool			exists(const std::string&);
		static bool			isDirectory(const std::string&);
		static bool			createDirectory(const std::string&);
};
