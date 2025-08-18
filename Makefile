NAME = webserv
SOURCE = 
OBJ = $(SOURCE:.cpp=.o)
FLAGS = -Wall -Wextra -Werror -std=c++98
SANITIZE = -fsanitize=address
CC = c++

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $^ -o $(NAME)

%.o:%.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f *.o

fclean: clean
	rm -f $(NAME)

re: fclean all

debug:
	$(CC) $(SANITIZE) $(FLAGS) $(SOURCE) -o $(NAME)

jwtest:
	c++ $(SANITIZE) src/Backend/CGI.cpp src/Backend/Epoll.cpp src/Backend/Socket.cpp src/Backend/SocketRegistry.cpp src/Backend/IO.cpp src/Exceptions/Exceptions.cpp src/Utils/Utils.cpp

.PHONY: all clean fclean re debug jwtest
