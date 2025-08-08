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

.PHONY: all clean fclean re debugt the winners (A), which is size 150
