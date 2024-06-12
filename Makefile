NAME = webserv

SRC = 	src/main.cpp \
		src/request.cpp \
	    src/server.cpp \
		src/conf_parsing.cpp

OBJ = $(SRC:.c=.o)
#HEADERS = server.hpp \

CXXFLAGS = -Wall -Wextra -Werror -g3 -std=c++98
CXX = c++

all: $(NAME)

$(NAME): $(OBJ) #$(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

obj/%.o: %.cpp #$(HEADERS)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf obj

fclean: clean
	@rm -rf $(NAME)

re: fclean all