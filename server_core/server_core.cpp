#include <sys/socket.h>
#define BACKLOG 10

int main(void)
{
	int sockfd;
	struct sockaddr_in sockaddr;
	unsigned int port = 8080;
	int connectionfd;
	unsigned int addrlen;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		std::cerr << "Error: failed to create socket" << std::endl;
		return ( EXIT_FAILURE );
	}
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(port);
	addrlen = sizeof(addr);
	if (bind(sockfd, (struct sockaddr*)&sockaddr, addrlen) < 0)
	{
		std::cerr << "Error: failed to bind to port " << port << std::endl;
		return ( EXIT_FAILURE );
	}
	if (listen(sockfd, BACKLOG) < 0)
	{
		std::cerr << "Error: failed to listen on socket" << std::endl;
		return ( EXIT_FAILURE );
	}
	connectionfd = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
	if (connectionfd < 0)
	{
		std::cerr << "Error: failed to grab connection" << std::endl;
		return ( EXIT_FAILURE );
	}
}
