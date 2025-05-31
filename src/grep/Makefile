CC = gcc
CFLAGS =  -Wall -Werror -Wextra -pedantic -std=c11

all: s21_cat

s21_cat: Makefile s21_grep.h s21_grep.c
	$(CC) $(CFLAGS) s21_grep.c -o s21_grep

rebuild: clean all

clean:
	rm -f s21_grep


.PHONY: all rebuild clean