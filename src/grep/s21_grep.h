#ifndef S21_GREP_H_
#define S21_GREP_H_

#include <regex.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
  int e;  /* -e */
  int i;  /* -i */
  int v;  /* -v */
  int c;  /* -c */
  int l;  /* -l */
  int n;  /* -n */
  int h;  /* -h */
  int s;  /* -s */
  int o;  /* -o */
} Flags;

typedef struct {
  int match_found;  /* 1 if file had a match */
  int error;        /* 1 on processing error */
} Result;

Result grep_file(const char *filename, char **patterns, int pat_count,
                 Flags flags, int total_files);

#endif  /* S21_GREP_H_ */
