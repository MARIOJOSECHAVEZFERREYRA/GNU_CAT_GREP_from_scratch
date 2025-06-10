#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "s21_cat.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
  Flags flags = {0};
  int line_num = 1;
  int error_code = SUCCESS;
  error_code = parseInput(argc, argv, &flags);

  if (error_code != SUCCESS) {
    printUsage(argv[0]);
  } else {
    argc -= optind;
    argv += optind;
    if (argc == 0) {
      catFile(NULL, flags, &line_num);
    } else {
      for (int i = 0; i < argc; i++) {
        catFile(argv[i], flags, &line_num);
      }
    }
  }

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

  while ((opt = getopt_long(argc, argv, "benstvET", long_options, NULL)) != -1) {
    if (opt == '?') error_code = ERROR_INPUT;
    markFlags(flags, opt);
  }

  return error_code;
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

void catFile(const char *filename, Flags flags, int *line_num) {
  FILE *f = filename ? fopen(filename, "r") : stdin;
  if (!f) {
    perror(filename);
    return;
  }

  char *line = NULL;
  size_t bufn = 0;
  ssize_t len;
  int prev_blank = 0;

  while ((len = getline(&line, &bufn, f)) != -1) {
    int is_blank = (len == 1 && line[0] == '\n');
    int skip = (flags.s && is_blank && prev_blank);

    if (!skip) {
      if (flags.b) {
        if (!is_blank) {
          printf("%6d\t", (*line_num)++);
        }
      } else if (flags.n) {
        printf("%6d\t", (*line_num)++);
      }

      if (is_blank) {
        if (flags.e) putchar('$');
        putchar('\n');
      } else {
        for (ssize_t i = 0; i < len; ++i) {
          unsigned char c = (unsigned char)line[i];

          if (c == '\n') {
            if (flags.e) putchar('$');
            putchar('\n');
          } else if (c == '\t') {
            if (flags.t)
              fputs("^I", stdout);
            else
              putchar('\t');
          } else if (flags.v) {
            handleNonPrintable(c);
          } else {
            putchar(c);
          }
        }
      }
    }

    prev_blank = is_blank;
  }

  free(line);
  if (f != stdin) fclose(f);
}



void handleNonPrintable(unsigned char c) {
  if (c >= 128) {
    unsigned char ch = c & 0x7F;
    if (ch < 32) {
      fputs("M-^", stdout);
      putchar(ch + 64);
    } else if (ch == 127) {
      fputs("M-^?", stdout);
    } else {
      fputs("M-", stdout);
      putchar(ch);
    }
  } else if (c < 32) {
    putchar('^');
    putchar(c + 64);
  } else if (c == 127) {
    fputs("^?", stdout);
  } else {
    putchar(c);
  }
}


void printUsage(const char *programName) {
  fprintf(stderr,
          "Usage: %s [ -b | --number-nonblank ] [ -n | --number ] "
          "[ -s | --squeeze-blank ] [ -e ] [ -E | --show-ends ] "
          "[ -t ] [ -T | --show-tabs ] [ -v | --show-nonprinting ] "
          "[files...]\n", programName);
}

