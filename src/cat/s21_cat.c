#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "s21_cat.h"

#include <stdlib.h>

//CAMBIAR, ESTO SI ES UNA VARIABLE GLOBAL
static int line_num = 1; //special for multiple files

int main(int argc, char *argv[]) {
    Flags flags = {0};
    int opt;
    while ((opt = getopt_long(argc, argv, "benstvET", long_options, NULL)) != -1) {
        switch (opt) {
            case 'b': flags.b = 1;            break;
            case 'n': flags.n = 1;            break;
            case 's': flags.s = 1;            break;
            case 'e': /* -e = -v + -E */
                flags.e = flags.v = 1;        break;
            case 'E': /* show-ends */
                flags.e = 1;                  break;
            case 't': /* -t = -v + -T */
                flags.t = flags.v = 1;        break;
            case 'T': /* show-tabs */
                flags.t = 1;                  break;
            case 'v': /* show-nonprinting */
                flags.v = 1;                  break;
            default:
                fprintf(stderr,
                    "Usage: %s [ -b | --number-nonblank ] [ -n | --number ] "
                    "[ -s | --squeeze-blank ] [ -e ] [ -E | --show-ends ] "
                    "[ -t ] [ -T | --show-tabs ] [ -v | --show-nonprinting ] [files...]\n",
                    argv[0]);
                return 1;
        }
    }

    argc -= optind;
    argv += optind;

    line_num = 1; //it initializes only once

    if (argc == 0) {
        cat_file(NULL, flags);
    } else {
        for (int i = 0; i < argc; i++) {
            cat_file(argv[i], flags);
        }
    }
    return 0;
}

void cat_file(const char *filename, Flags flags) {
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
        int skip     = (flags.s && is_blank && prev_blank);

        if (!skip) {
            if (flags.b) {
                if (!is_blank) {
                    printf("%6d\t", line_num++);
                }
            } else if (flags.n) {
                printf("%6d\t", line_num++);
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
                        if (flags.t)      fputs("^I", stdout);
                        else              putchar('\t');

                    } else if (flags.v) {
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
