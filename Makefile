
CC = c++
FLAGS = -std=c++98 -Wall -Wextra -Werror

SRC_DIR = src/
INC_DIR = include/
TMP_DIR = tmp/


SRCS = irc.cpp user.cpp
OBJS = $(addprefix $(TMP_DIR), $(SRCS:.cpp=.o))
DEPS = $(OBJS:.o=.d)

NAME = ircserv

DFLAGS = -MMD -MP

all: $(TMP_DIR) $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

$(TMP_DIR):
	@mkdir -p $(TMP_DIR)

$(TMP_DIR)%.o: $(SRC_DIR)%.cpp
	$(CC) $(FLAGS) $(DFLAGS) -I $(INC_DIR) -c $< -o $@

clean:
	rm -rf $(TMP_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean all re

-include $(DEPS)
