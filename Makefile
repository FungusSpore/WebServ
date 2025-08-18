NAME = Webserve

SRC_DIR = src
INC_DIR = includes

SRC = $(shell find $(SRC_DIR) -type f -name '*.cpp')

OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(SRC_DIR)/%.o)

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
IFLAGS = -I$(INC_DIR)

all: $(NAME)

$(NAME): $(OBJ)
	@echo "Creating $(NAME)..."
	@$(CC) $(CFLAGS) $(IFLAGS) $(OBJ) -o $(NAME) 
	@echo "Created $(NAME)"

bonus: all

%.o: %.cpp
	@$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

clean:
	@echo "Removing objects files..."
	@rm -f $(OBJ)
	@echo "Objects files removed."

fclean: clean
	@echo "Removing $(NAME)..."
	@rm -f $(NAME)
	@echo "$(NAME) removed."

re: fclean all

debug: CFLAGS += -g3 -fpic
debug: clean all
	@make -C $(LIBFT_DIR) debug
	@echo "Enable debug mode"

jwtest:
	c++ $(SANITIZE) $(FLAGS) src/Backend/CGI.cpp src/Backend/Epoll.cpp src/Backend/Socket.cpp src/Backend/SocketRegistry.cpp src/Backend/IO.cpp src/Exceptions/Exceptions.cpp src/Utils/Utils.cpp

valgrind:
	valgrind --leak-check=full ./$(NAME) dummy_server.conf

fval:
	valgrind -v --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) dummy_server.conf

fsanitize: CFLAGS += -fsanitize=address,leak -g3 -O1
fsanitize: clean all
	@echo "AddressSanitizer enabled for $(NAME)"

fsan: fsanitize

.PHONY: all clean fclean re bonus debug valgrind fsanitize fsan fval jwtest
