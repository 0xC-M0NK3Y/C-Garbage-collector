NAME = gcollector
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g
LDFLAGS = -lpthread
SRC = $(addprefix src/, main.c  gcollector.c )
OBJ = ${SRC:.c=.o}

all : ${NAME}

${NAME}: ${OBJ}
		${CC} ${OBJ} ${CFLAGS} ${LDFLAGS} -o ${NAME}

%.o: %.c
		${CC} -c ${<} -o ${@} ${CFLAGS}

clean:
		rm -rf ${OBJ}

fclean: clean
		rm -rf ${NAME}

re : fclean all
