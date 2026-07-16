NAME        = ircserv

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98

INCLUDES    = -I./include

INCLUDES_   =  \
	include/Channel.hpp \
	include/Client.hpp \
	include/Parser.hpp \
	include/Server.hpp 


SRC         =  \
	src/main.cpp \
	src/server.cpp \
	src/utils_server.cpp \
	src/init_server.cpp \
	src/nick_user_pass.cpp \
	src/Client.cpp \
	src/Channel.cpp \
	src/Parser.cpp \
	src/commands/JoinCommand.cpp \
	src/commands/ChannelUtils.cpp \
	src/commands/TopicCommand.cpp

OBJ         = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ) 
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(INCLUDES_)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
