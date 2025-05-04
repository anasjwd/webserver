# pragma once

# include <map>
# include <string>

# define	HTTP_OK									200
# define	HTTP_BAD_REQUEST						400
# define	HTTP_UNAUTHORIZED						401
# define	HTTP_PAYMENT_REQUIRED					402
# define	HTTP_FORBIDDEN							403
# define	HTTP_NOT_FOUND							404
# define	HTTP_METHOD_NOT_ALLOWED					405
# define	HTTP_NOT_ACCEPTABLE						406
# define	HTTP_PROXY_AUTHENTICATION_REQUIRED		407
# define	HTTP_REQUEST_TIMEOUT					408
# define	HTTP_CONFLICT							409
# define	HTTP_GONE								410
# define	HTTP_LENGTH_REQUIRED					411
# define	HTTP_PRECONDITION_FAILED				412
# define	HTTP_PAYLOAD_TOO_LARGE					413
# define	HTTP_URI_TOO_LONG						414
# define	HTTP_UNSUPPORTED_MEDIA_TYPE				415
# define	HTTP_RANGE_NOT_SATISFIABLE				416
# define	HTTP_EXPECTATION_FAILED					417
# define	HTTP_UPGRADE_REQUIRED					426
# define	HTTP_INTERNAL_SERVER_ERROR				500
# define	HTTP_NOT_IMPLEMENTED					501
# define	HTTP_BAD_GATEWAY						502
# define	HTTP_SERVICE_UNAVAILABLE				503
# define	HTTP_GATEWAY_TIMEOUT					504
# define	HTTP_VERSION_NOT_SUPPORTED				505

class   Request
{

	public:
		Request();
		~Request();

		std::string							uri;
		std::string							body;
		bool								close;
		int									state;
		std::string							method;
		std::string							version;

		std::map<std::string, std::string>	headers;
		std::map<std::string, std::string>	bodyParams;
		std::map<std::string, std::string>	queryParams;

		bool								parseUri();
		bool								parseMethod();
		bool								parseVersion();
		bool								parseHeaders(std::stringstream& stream, std::string& line);

		bool								parseJsonBody();
		bool								parseUrlEncodedBody();
		bool								parseBodyByContentType();
		bool								parseMultipartBody(std::string& type);

		void								clearRequest();
		void								readFromSocket(int sockfd);
		bool								readBodyFromBuffer(std::string& buffer);
		bool								parseRequestLineAndHeaders(std::string& buffer);
};
