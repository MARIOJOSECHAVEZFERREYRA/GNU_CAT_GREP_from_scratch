#ifndef _S21_CAT_H_
#define _S21_CAT_H_

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

typedef struct {
    int b, e, n, s, t, v;
} Flags;


void cat_file(const char *filename, Flags flags, int *line_num);

#endif