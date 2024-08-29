# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: galambey <galambey@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/07/09 11:44:01 by galambey          #+#    #+#              #
#    Updated: 2024/08/29 14:40:47 by galambey         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

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
					check_content.cpp \
					init_err_pgs.cpp \
					adapt_host.cpp \

SERV_DIR		=	server/
SERV_SRCS		=	Server.cpp \
					close_server.cpp \
					Listen.cpp \
					Media.cpp

EXCEPTION_DIR	=	exception/
EXCEPTION_SRCS	=	ServerException.cpp \
					NotAnIntException.cpp

REQUEST_DIR		=	request/
REQUEST_SRCS	=	Request.cpp \
					Response.cpp

ERROR_DIR		=	error/
ERROR_SRCS		=	ErrorPages.cpp

UTILS_DIR		=	utils/
UTILS_SRCS		=	utils.cpp

############################# HANDLE DIRECTORIES ##############################

SRCS_DIR 		= 	src/

SRCS			=	$(addprefix $(MAIN_DIR), $(MAIN_SRCS)) \
					$(addprefix $(CONF_DIR), $(CONF_SRCS)) \
					$(addprefix $(SERV_DIR), $(SERV_SRCS)) \
					$(addprefix $(EXCEPTION_DIR), $(EXCEPTION_SRCS)) \
					$(addprefix $(REQUEST_DIR), $(REQUEST_SRCS)) \
					$(addprefix $(ERROR_DIR), $(ERROR_SRCS)) \
					$(addprefix $(UTILS_DIR), $(UTILS_SRCS))

OBJS_DIR 		= 	.objs/

OBJS_NAMES 		= 	$(SRCS:.cpp=.o)

OBJS			= 	$(addprefix $(OBJS_DIR), $(OBJS_NAMES))

DEPS 		:= $(OBJS:.o=.d)

#################################### RULES ####################################

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

$(OBJS_DIR)%.o:$(SRCS_DIR)%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean :
	rm -rf $(OBJS_DIR)
	rm -f $(DEPS)

fclean : clean
	rm -f $(NAME)

re : fclean
	make all

.PHONY : all clean fclean re
