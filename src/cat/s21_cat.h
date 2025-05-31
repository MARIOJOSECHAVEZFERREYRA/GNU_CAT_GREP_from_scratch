#ifndef _S21_CAT_H_
#define _S21_CAT_H_

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

typedef struct {
    int b, e, n, s, t, v;
} Flags;

static struct option long_options[] = {
    {"number-nonblank", no_argument,       0, 'b'},
    {"number",          no_argument,       0, 'n'},
    {"squeeze-blank",   no_argument,       0, 's'},
    {"show-ends",       no_argument,       0, 'E'},
    {"show-nonprinting",no_argument,       0, 'v'},
    {"show-tabs",       no_argument,       0, 'T'},
    {0, 0, 0, 0}
};

void cat_file(const char *filename, Flags flags);

#endif
