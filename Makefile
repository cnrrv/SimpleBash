CC = gcc
CFLAGS = -Wall -Werror -Wextra

all: clean s21_grep test

s21_grep:
	$(CC) $(FLAGS) grep3.c -o s21_grep

test:
	bash test_grep.sh

clean:
	rm -f s21_grep