NAME 			= 	webserv
CXX 			= 	c++
CXXFLAGS		= 	-Wall -Wextra -Werror -g3 -MMD -std=c++98

################################### SOURCES ###################################

MAIN_DIR		=	main/
MAIN_SRCS		=	main.cpp

CONF_DIR		=	configuration_file/
CONF_SRCS		=	conf_parsing.cpp \
					check_syntax.cpp \
					init_conf_struct.cpp \
					init_location.cpp \
					conf_utils.cpp \

SERV_DIR		=	server/
SERV_SRCS		=	server.cpp \
					close_server.cpp \
					handle_gate.cpp

EXCEPTION_DIR	=	exception/
EXCEPTION_SRCS	=	ServerException.cpp
					
REQUEST_DIR		=	request/
REQUEST_SRCS	=	request.cpp

ERROR_DIR		=	error/
ERROR_SRCS		=	handle_http_error.cpp

A_EFFACER_DIR		=	a_effacer/
A_EFFACER_SRCS	=	print.cpp

############################# HANDLE DIRECTORIES ##############################

SRCS_DIR 		= 	src/

SRCS			=	$(addprefix $(MAIN_DIR), $(MAIN_SRCS)) \
					$(addprefix $(CONF_DIR), $(CONF_SRCS)) \
					$(addprefix $(SERV_DIR), $(SERV_SRCS)) \
					$(addprefix $(EXCEPTION_DIR), $(EXCEPTION_SRCS)) \
					$(addprefix $(REQUEST_DIR), $(REQUEST_SRCS)) \
					$(addprefix $(ERROR_DIR), $(ERROR_SRCS)) \
					$(addprefix $(A_EFFACER_DIR), $(A_EFFACER_SRCS))

OBJS_DIR 		= 	.objs/

OBJS_NAMES 		= 	$(SRCS:.cpp=.o)

OBJS			= 	$(addprefix $(OBJS_DIR), $(OBJS_NAMES))

DEPS 		:= $(OBJS:.o=.d)

#################################### RULES ####################################

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXX_FLAGS) $(OBJS) -o $@

$(OBJS_DIR)%.o:$(SRCS_DIR)%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -c $< -o $@ $(MLX_FLAGS)

-include $(DEPS)

clean :
	rm -rf $(OBJS_DIR)
	rm -f $(DEPS)

fclean : clean
	rm -f $(NAME)

re : fclean
	make all

.PHONY : all clean fclean re