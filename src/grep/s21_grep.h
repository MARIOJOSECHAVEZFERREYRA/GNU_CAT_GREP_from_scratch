#ifndef _S21_GREP_H_
#define _S21_GREP_H_

#include <stdio.h>
#include <unistd.h>
#include <regex.h>

typedef struct {
    int e;      // -e
    int i;      // -i
    int v;      // -v
    int c;      // -c
    int l;      // -l
    int n;      // -n
    int h;      // -h
    int s;      // -s:
    int o;      // -o
} Flags;

void grep_file(const char *filename, char **patterns, int pat_count,
               Flags flags, int total_files);

#endif // _S21_GREP_H_
