NAME		=	Webserv

CXX			=	c++ -std=c++98
CFLAGS		=	-Wall -Wextra -Werror

RM			=	rm -f

SRC			=	$(wildcard */*.cpp *.cpp)
OBJ			=	$(SRC:.cpp=.o)

all:		$(NAME)

$(NAME):	$(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	$(RM) $(OBJ)

fclean:	clean
	$(RM) $(NAME)

re:	fclean all

test:
	@echo "=>" $(SRC);