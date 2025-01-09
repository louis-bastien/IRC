CC = c++
FLAGS = -std=c++98 -Wall -Wextra -Werror 

SRC_DIR = src/
INC_DIR = include/
TMP_DIR = tmp/
LOG_DIR = log/


SRCS = irc.cpp User.cpp Logger.cpp Server.cpp Message.cpp MessageHandler.cpp Channel.cpp
OBJS = $(addprefix $(TMP_DIR), $(SRCS:.cpp=.o))
DEPS = $(OBJS:.o=.d)

NAME = ircserv

DFLAGS = -MMD -MP

all: $(TMP_DIR) $(LOG_DIR) $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

$(TMP_DIR):
	@mkdir -p $(TMP_DIR)

$(LOG_DIR):
	@mkdir -p $(LOG_DIR)

$(TMP_DIR)%.o: $(SRC_DIR)%.cpp
	$(CC) $(FLAGS) $(DFLAGS) -I $(INC_DIR) -c $< -o $@

clean:
	rm -rf $(TMP_DIR)
	rm -rf $(LOG_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean all re

-include $(DEPS)
