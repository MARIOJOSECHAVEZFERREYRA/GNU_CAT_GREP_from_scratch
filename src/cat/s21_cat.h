#ifndef _S21_CAT_H_
#define _S21_CAT_H_


#define SUCCESS 0
#define ERROR_INPUT -1
#define ERROR_FILE_READ -2

#include <stdio.h>

typedef struct {
    int b, e, n, s, t, v;
} Flags;


int parseInput(int argc, char* argv[], Flags* flags);
void markFlags(Flags* flags, int opt);
void catFile(FILE* file, Flags* flags);
void printNonPrinting(int c);
void usage();
#endif