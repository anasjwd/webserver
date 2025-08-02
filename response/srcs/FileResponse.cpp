#include "../include/FileResponse.hpp"
#include "../include/ErrorResponse.hpp"
#include <sys/stat.h>

Response FileResponse::serve(const std::string& filePath, const std::string& mimeType, int statusCode) {
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0 || !S_ISREG(fileStat.st_mode)) {
        // return ErrorResponse::createNotFoundResponse(conn) TODO ;
        return Response(404);
    }
    Response response(statusCode);
    response.setFilePath(filePath);
    response.setContentType(mimeType);
    response.setFileSize(static_cast<size_t>(fileStat.st_size));
    response.addHeader("Connection", "close");
    return response;
} 