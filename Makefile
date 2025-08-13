CXX = c++ -std=c++98
CXXFLAGS = -Wall -Wextra -Werror
TARGET = webserv

SRCS     = conf/AutoIndex.cpp conf/BlockDirective.cpp conf/cfg_parser.cpp conf/ClientMaxBodySize.cpp conf/DirectiveFactory.cpp conf/DirectiveFactoryUtils.cpp \
		 conf/ErrorPage.cpp conf/Exceptions.cpp conf/Http.cpp conf/Index.cpp conf/LimitExcept.cpp conf/Listen.cpp conf/Location.cpp conf/parser.cpp conf/Return.cpp \
		 conf/Root.cpp conf/Server.cpp conf/ServerName.cpp conf/tokenizer.cpp conf/Upload.cpp conf/UploadLocation.cpp request/srcs/FileHandler.cpp \
		 request/srcs/RequestBody.cpp request/srcs/Request.cpp request/srcs/RequestHeaders.cpp request/srcs/RequestLine.cpp response/srcs/CgiHandler.cpp \
		 response/srcs/ErrorResponse.cpp response/srcs/FileResponse.cpp response/srcs/MimeTypes.cpp response/srcs/Response.cpp \
		 response/srcs/ResponseHandler.cpp response/srcs/ResponseSender.cpp response/srcs/ReturnHandler.cpp Connection.cpp server_core.cpp
OBJS     = $(SRCS:.cpp=.o)


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re

.SECONDARY: $(OBJS)
