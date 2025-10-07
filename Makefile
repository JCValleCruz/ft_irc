NAME = ircserv
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address
CC = c++
OBJDIR = obj

SRCS := main.cpp Server.cpp Server_Utils.cpp Client.cpp _pass.cpp \
_nick.cpp _join.cpp _user.cpp Channel.cpp _cap.cpp _quit.cpp _kick.cpp _mode.cpp _part.cpp \
_privmsg.cpp _invite.cpp _topic.cpp _mode1.cpp 

OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
		$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: %.cpp
		@mkdir -p $(OBJDIR)
		$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(NAME)

re : fclean all

.PHONY: all clean fclean re