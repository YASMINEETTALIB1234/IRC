# **************************************************************************** #
#                                   VARIABLES                                  #
# **************************************************************************** #

NAME        = ircserv

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98

INCLUDES    = -I./include

SRC         =  \
	src/main.cpp \
	src/server.cpp \
	src/Client.cpp \
	src/Channel.cpp \
	src/Parser.cpp \
	src/commands/JoinCommand.cpp \
	src/commands/ChannelUtils.cpp

OBJ         = $(SRC:.cpp=.o)

# **************************************************************************** #
#                                    RULES                                     #
# **************************************************************************** #

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re