#ifndef _S21_CAT_H_
#define _S21_CAT_H_

#include <getopt.h>
#include <stdio.h>
#include <unistd.h>

#define SUCCESS 0
#define ERROR_INPUT -1
#define ERROR_FILE_READ -2

typedef struct {
  int b, e, n, s, t, v;
} Flags;

void catFile(const char *filename, Flags flags, int *line_num);
int parseInput(int argc, char* argv[], Flags* flags);
void markFlags(Flags* flags, int opt);
void handleNonPrintable(unsigned char c);
void printUsage(const char *programName);
#endif
