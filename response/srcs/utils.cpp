#include "../include/utils.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <iostream>

std::string loadFile(const std::string& path) {
    std::string dir = "tmp/response";
    struct stat st;
    memset(&st, 0, sizeof(st));
    if (stat(dir.c_str(), &st) == -1) {
        mkdir("tmp", 0755);
        mkdir(dir.c_str(), 0755);
    }

    if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
        return "";
    if (access(path.c_str(), R_OK) != 0)
        return "";

    std::srand(std::time(0) ^ getpid());
    std::stringstream ss;
    ss << dir << "/resp_" << std::rand() << ".tmp";
    std::string filename = ss.str();

    int srcFd = open(path.c_str(), O_RDONLY);
    if (srcFd == -1)
        return "";
    int dstFd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dstFd == -1) {
        close(srcFd);
        return "";
    }
    char buf[1048576];
    ssize_t bytesRead;
    while ((bytesRead = read(srcFd, buf, sizeof(buf))) > 0) {
        ssize_t totalWritten = 0;
        while (totalWritten < bytesRead) {
            ssize_t bytesWritten = write(dstFd, buf + totalWritten, bytesRead - totalWritten);
            std::cout << "write:utils.cpp -> " << bytesWritten << "\n";
            if (bytesWritten == -1) {
                close(srcFd);
                close(dstFd);
                unlink(filename.c_str());
                return "";
            }
            totalWritten += bytesWritten;
        }
    }
    close(srcFd);
    close(dstFd);
    if (bytesRead < 0) {
        unlink(filename.c_str());
        return "";
    }
    return filename;
}

std::string getMimeType(const std::string& path) {
	if (path.find(".html") != std::string::npos) return "text/html";
	if (path.find(".css") != std::string::npos) return "text/css";
	if (path.find(".js") != std::string::npos) return "application/javascript";
	if (path.find(".png") != std::string::npos) return "image/png";
	return "text/plain";
}
