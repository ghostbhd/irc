# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ohrete <ohrete@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/04/08 00:06:29 by ohrete            #+#    #+#              #
#    Updated: 2023/06/05 23:36:52 by ohrete           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

CPPFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp server.cpp

OBJS = ${SRCS:.cpp=.o}

all : ${NAME}

${NAME} : ${OBJS}
	c++ ${CPPFLAGS} ${OBJS} -o ${NAME}

clean :
	rm -rf ${OBJS}
	
fclean : clean
	rm -rf ${NAME}

re : fclean all