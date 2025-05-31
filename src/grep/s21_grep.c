#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "s21_grep.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Para ordenar rangos de match
typedef struct { size_t so, eo; } Match;
static int compare_match(const void *a, const void *b) {
    const Match *A = a, *B = b;
    return (A->so < B->so) ? -1 : (A->so > B->so);
}

static char** load_patterns_from_file(const char *fname, int *out_count) {
    FILE *f = fopen(fname, "r");
    if (!f) return NULL;
    char **pats = NULL;
    size_t cap = 0;
    *out_count = 0;
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, f) != -1) {
        // quitar salto de línea
        if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
        if (*out_count >= (int)cap) {
            cap = cap ? cap*2 : 16;
            pats = realloc(pats, cap * sizeof(char*));
        }
        pats[(*out_count)++] = strdup(line);
    }
    free(line);
    fclose(f);
    return pats;
}

int main(int argc, char *argv[]) {
    Flags flags = {0};
    int opt;
    char **patterns = malloc(argc * sizeof(char*));
    int pat_count = 0;

    while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
        switch (opt) {
            case 'e':
                patterns[pat_count++] = optarg; flags.e = 1;
                break;
            case 'i': flags.i = 1; break;
            case 'v': flags.v = 1; break;
            case 'c': flags.c = 1; break;
            case 'l': flags.l = 1; break;
            case 'n': flags.n = 1; break;
            case 'h': flags.h = 1; break;
            case 's': flags.s = 1; break;
            case 'o': flags.o = 1; break;
            case 'f': {
                flags.e = 1;
                int count = 0;
                char **fromf = load_patterns_from_file(optarg, &count);
                if (!fromf) {
                    if (!flags.s) perror(optarg);
                    continue;
                }
                for (int i = 0; i < count; i++)
                    patterns[pat_count++] = fromf[i];
                free(fromf);
                break;
            }
            default:
                fprintf(stderr,
                    "Usage: %s [-e pattern] [-f file] [-i] [-v] [-c] [-l] [-n] [-h] [-s] [-o] [pattern] [file...]\n",
                    argv[0]);
                return 1;
        }
    }

    argc -= optind; argv += optind;
    if (pat_count == 0) {
        if (argc > 0) {
            patterns[pat_count++] = argv[0];
            argv++; argc--;
        } else {
            fprintf(stderr, "grep: missing pattern\n");
            return 1;
        }
    }

    int total_files = argc;
    if (total_files == 0) {
        grep_file(NULL, patterns, pat_count, flags, total_files);
    } else {
        for (int i = 0; i < total_files; i++) {
            grep_file(argv[i], patterns, pat_count, flags, total_files);
        }
    }
    // liberamos patrones duplicados por -f
    for (int i = 0; i < pat_count; i++)
        if (flags.e && patterns[i] && strdup(patterns[i]) != patterns[i])
            free(patterns[i]);
    free(patterns);
    return 0;
}

void grep_file(const char *filename, char **patterns, int pat_count,
               Flags flags, int total_files) {
    FILE *f = filename ? fopen(filename, "r") : stdin;
    if (!f) {
        if (!flags.s) perror(filename);
        return;
    }

    // compilamos todos los patrones
    regex_t *regs = malloc(pat_count * sizeof(regex_t));
    int regc_flags = flags.i ? REG_ICASE : 0;
    for (int p = 0; p < pat_count; p++) {
        if (regcomp(&regs[p], patterns[p], regc_flags) != 0) {
            fprintf(stderr, "Invalid regex: %s\n", patterns[p]);
            while (p--) regfree(&regs[p]);
            free(regs);
            if (f != stdin) fclose(f);
            return;
        }
    }

    char *line = NULL; size_t len = 0;
    ssize_t read;
    int   line_no      = 1;
    int   match_count  = 0;
    int   matched_file = 0;

    while ((read = getline(&line, &len, f)) != -1) {
        // recogemos todos los matches de todos los patrones
        Match *matches = malloc(sizeof(Match) * 128);
        int   mcap    = 128, mcount = 0;

        for (int p = 0; p < pat_count; p++) {
            char *cursor = line;
            regmatch_t pm;
            while (regexec(&regs[p], cursor, 1, &pm, 0) == 0) {
                size_t so = (cursor - line) + pm.rm_so;
                size_t eo = (cursor - line) + pm.rm_eo;
                if (mcount >= mcap) {
                    mcap *= 2;
                    matches = realloc(matches, mcap * sizeof(Match));
                }
                matches[mcount++] = (Match){so, eo};
                cursor += pm.rm_eo;
            }
        }

        int any_match = (mcount > 0);
        if (flags.v) any_match = !any_match;
        if (any_match) {
            matched_file = 1;
            match_count++;
        }

        // impresión: ni -c ni -l
        if (any_match && !flags.c && !flags.l) {
            // prefijos
            if (!flags.h && filename && total_files > 1)
                printf("%s:", filename);
            if (flags.n)
                printf("%d:", line_no);

            if (flags.o && !flags.v) {
                // solo coincidencias, una por línea
                for (int i = 0; i < mcount; i++) {
                    printf("%.*s\n",
                        (int)(matches[i].eo - matches[i].so),
                        line + matches[i].so);
                }
            } else if (flags.v) {
                fputs(line, stdout);
            } else {
                // resaltado sobre línea completa
                qsort(matches, mcount, sizeof(Match), compare_match);
                size_t cur = 0;
                for (int i = 0; i < mcount; i++) {
                    printf("%.*s", (int)(matches[i].so - cur), line + cur);
                    printf("\033[31m%.*s\033[0m",
                        (int)(matches[i].eo - matches[i].so),
                        line + matches[i].so);
                    cur = matches[i].eo;
                }
                printf("%s", line + cur);
            }
        }
        free(matches);
        line_no++;
    }

    // -c
    if (flags.c) {
        if (!flags.h && filename && total_files > 1)
            printf("%s:", filename);
        printf("%d\n", match_count);
    }
    // -l
    if (flags.l && matched_file && filename) {
        printf("%s\n", filename);
    }

    for (int p = 0; p < pat_count; p++)
        regfree(&regs[p]);
    free(regs);
    free(line);
    if (f != stdin) fclose(f);
}