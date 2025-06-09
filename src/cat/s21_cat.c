#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "s21_cat.h"

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  int error_code = SUCCESS;
  Flags flags = {0};
  int fileArgumentIndex = ERROR_INPUT;

  if (argc < 1 || (fileArgumentIndex = parseInput(argc,argv, &flags)) == -1 )
    error_code = ERROR_INPUT;
  else {
    FILE* file = NULL;
    if (argc == fileArgumentIndex) file = stdin;
    while (error_code == SUCCESS && (fileArgumentIndex < argc || file == stdin)) {
      if (file != stdin && (file = fopen(argv[fileArgumentIndex], "r")) == NULL) {
        perror(argv[fileArgumentIndex]);
        continue;
      }
      catFile(file, &flags);
      fclose(file);
      fileArgumentIndex++;
    }
  }
  if (error_code == ERROR_INPUT) usage();
  return error_code;
}

int parseInput(int argc, char* argv[], Flags* flags) {
  int error_code = SUCCESS;
  int opt;
  static struct option long_options[] = {
      {"number-nonblank", no_argument, 0, 'b'},
      {"number", no_argument, 0, 'n'},
      {"squeeze-blank", no_argument, 0, 's'},
      {"show-ends", no_argument, 0, 'E'},
      {"show-nonprinting", no_argument, 0, 'v'},
      {"show-tabs", no_argument, 0, 'T'},
      {0, 0, 0, 0}};

  while ((opt = getopt_long(argc, argv, "benstvET", long_options, NULL)) !=
         -1) {
    if (opt == '?') error_code = ERROR_INPUT;
    markFlags(flags, opt);
  }
  return error_code == SUCCESS ? optind : error_code;
}
void markFlags(Flags* flags, int opt) {
  switch (opt) {
    case 'b':
      flags->b = 1;
      break;
    case 'n':
      flags->n = 1;
      break;
    case 's':
      flags->s = 1;
      break;
    case 'e': /* -e = -v + -E */
      flags->e = flags->v = 1;
      break;
    case 'E': /* show-ends */
      flags->e = 1;
      break;
    case 't': /* -t = -v + -T */
      flags->t = flags->v = 1;
      break;
    case 'T': /* show-tabs */
      flags->t = 1;
      break;
    case 'v': /* show-nonprinting */
      flags->v = 1;
      break;
  }
}
void catFile(FILE* file, Flags* flags) {
  int isPrevBlank = 0;          // for -s
  int ch, prevCh, counter = 1;  // counter  for -n or -b
  for (prevCh = '\n'; (ch = fgetc(file)) != EOF; prevCh = ch) {
    // if prevCh is \n is because we are at the beggining of the new line
    if (prevCh == '\n') {
      // for cases where there are a lot of linea breaks
      if (flags->s) {
        if (ch == '\n') {
          if (isPrevBlank) continue;
          isPrevBlank = 1;
        } else
          isPrevBlank = 0;
      }
      // line counting
      if (flags->b) {
        if (ch != '\n')
          printf("%6d\t", counter++);
        else if (flags->e)
          printf("%6s\t", "");  // reserve a space 6 spaces + tab for $
      } else if (flags->n)
        printf("%6d\t", counter++);
    }
    // for -e, -t and -v cases
    if (ch == '\n') {
      if (flags->e) putchar('$');
    } else if (ch == '\t') {
      if (flags->t) {
        fputs("^I", stdout);
        continue;
      }
    } else if (!isprint(ch)) {
      if (flags->v) {
        printNonPrinting(ch);
        continue;
      }
    }
    putchar(ch);  // final print
  }
}

void printNonPrinting(int c) {
  if (c & ~0x7F) {
    fputs("M-", stdout);
    c &= 0x7F;
  }
  if (iscntrl(c)) {
    c = (c == '\177') ? '?' : (c | 0100);
    putchar('^');
  }
  putchar(c);
}

void usage() {
  printf(
      "usage: [-benstv] [file...]\n"
      "  -b  number nonempty lines\n"
      "  -e  show $ at end of each line\n"
      "  -n  number all lines\n"
      "  -s  squeeze blank lines\n"
      "  -t  show tabs as ^I\n"
      "  -v  show nonprinting as ^X\n");
}