SHELL := /bin/zsh #using zsh shell

# Variables -------------------------------------------
SRCS =	*.cpp
CPP = @c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
RM = @rm -rf
AR = @ar -rc
NAME = ircserv
sanitize = -fsanitize=address -static-libasan -g3

# Colors ----------------------------------------------
RESET := "\x1b[0m"
BOLD := "\x1b[1m"
BLACK := "\x1b[30m"
RED := "\x1b[31m"
GREEN := "\x1b[32m"
YELLOW := "\x1b[33m"
BLUE := "\x1b[34m"
MAGENTA := "\x1b[35m"
CYAN := "\x1b[36m"
WHITE := "\x1b[37m"
#------------------------------------------------------

all : $(NAME)

$(NAME) : ${SRCS}
	${CPP} ${CFLAGS} $(SRCS) -o $(NAME)
	@echo $(BOLD)$(GREEN)"\n✅\tMandatory Compele\n\t"\
	$(WHITE)"Programe - "$(YELLOW)"($(NAME))\n" $(RESET)

clean :
	${RM} $(NAME)
	@echo $(BOLD)$(RED)"\n✅\tDelete" $(WHITE)"[$(NAME)]\n" $(RESET)

re : clean all