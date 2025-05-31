CC = gcc
CFLAGS =  -Wall -Werror -Wextra -pedantic -std=c11

all: s21_cat

s21_cat: Makefile s21_cat.h s21_cat.c
	$(CC) $(CFLAGS) s21_cat.c -o s21_cat

rebuild: clean all

clean:
	rm -f s21_cat


.PHONY: all rebuild clean