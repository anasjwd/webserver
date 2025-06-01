CXX = c++ -std=c++98
CXXFLAGS = -Wall -Wextra -Werror -g3 -fsanitize=address
TARGET = conf
SRCS     = $(wildcard *.cpp)
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
