// ========================================================================
//  s21_grep.c
//
//  • Returns 0 if at least one match was found.
//  • Returns 1 if it finished without finding any matches.
//  • Returns 2 if there was a usage error (e.g. -e without an argument, -f
//  unable to open)
//    or if a file could not be opened (unless -s is used).
// ========================================================================
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "s21_grep.h"

#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int exit_status = G_NO_MATCH;
  LinesList patterns = {0, NULL};
  Flags flags = {0};

  int first_file = parseInput(argc, argv, &flags, &patterns);
  regex_t regexes[patterns.count];
  if (argc < 2 || first_file < 0) {
    printUsage(argv[0]);
    exit_status = G_USAGE_ERROR;
  } else {
    int total_files = argc - first_file;
    if (total_files == 0) {
      fprintf(stderr, "error: at least one file is needed\n");
      exit_status = G_USAGE_ERROR;
    } else {
      // compile patterns
      if (prepareRegex(&patterns, regexes, &flags) != G_SUCCESS) {
        exit_status = G_USAGE_ERROR;
      } else {
        // proccess each file
        for (int i = first_file; i < argc && exit_status != G_USAGE_ERROR;
             i++) {
          int match_count = 0;
          int rc = grepFile(argv[i], &patterns, regexes, flags, total_files,
                            &match_count);
          if (rc == G_USAGE_ERROR) {
            exit_status = G_USAGE_ERROR;
          } else if (match_count > 0) {
            exit_status = G_SUCCESS;
          }
        }
      }
    }
  }

  for (size_t i = 0; i < patterns.count; i++) {
    regfree(&regexes[i]);
  }
  freeLinesList(&patterns);
  return exit_status;
}

int grepFile(const char* filename, LinesList* patterns, regex_t regexes[],
             Flags flags, int total_files, int* out_match_count) {
  int err_code = G_SUCCESS;
  *out_match_count = 0;
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    if (!flags.s) perror(filename);
    err_code = G_USAGE_ERROR;
  } else {
    LinesList selected_lines = {0, NULL};
    err_code = extractMatchingLines(file, &flags, &selected_lines,
                                    patterns->count, regexes);
    if (err_code == G_SUCCESS) {
      if (flags.l) {
        if (selected_lines.count > 0) {
          puts(filename);
        }
        *out_match_count = selected_lines.count;
      } else if (flags.c) {
        if (total_files > 1 && !flags.h) printf("%s:", filename);
        printf("%zu\n", selected_lines.count);
        *out_match_count = selected_lines.count;
      } else if (flags.o) {
        if (flags.v) {
          // -v -o: no imprimimos, pero contamos líneas NO coincidentes
          *out_match_count = selected_lines.count;
        } else {
          LinesList selected_substrings = {0, NULL};
          for (size_t i = 0; i < selected_lines.count && err_code == G_SUCCESS;
               i++) {
            int sub_err =
                extractMatches(&selected_lines.lines[i], &selected_substrings,
                               patterns->count, regexes);
            if (sub_err != G_SUCCESS) {
              err_code = sub_err;
            }
          }
          if (err_code == G_SUCCESS) {
            printSelectedLines(&flags, &selected_substrings,
                               (total_files > 1 && !flags.h) ? filename : NULL);
            *out_match_count = selected_substrings.count;
          }
          freeLinesList(&selected_substrings);
        }
      } else {
        printSelectedLines(&flags, &selected_lines,
                           (total_files > 1 && !flags.h) ? filename : NULL);
        *out_match_count = selected_lines.count;
      }
    }
    freeLinesList(&selected_lines);
    fclose(file);
  }
  return err_code;
}

//======================================================================================
// FOR INPUT FLAGS
//======================================================================================
int parseInput(int argc, char* argv[], Flags* flags, LinesList* patterns) {
  int result = G_SUCCESS;
  int opt;
  while (result == G_SUCCESS &&
         (opt = getopt(argc, argv, "e:f:chilnosv")) != -1) {
    if (opt == 'e') {
      result = addLineToLineList(patterns, optarg, 0);
    } else if (opt == 'f') {
      result = readPatternFromFile(patterns, optarg);
    } else if (opt == '?') {
      result = G_USAGE_ERROR;
    } else {
      markFlags(flags, opt);
    }
  }
  if (result == G_SUCCESS && patterns->count == 0)
    result = (optind == argc) ? G_SUCCESS
                              : addLineToLineList(patterns, argv[optind++], 0);
  return result == G_SUCCESS ? optind : result;
}
//======================================================================================
void markFlags(Flags* flags, int opt) {
  switch (opt) {
    case 'c':
      flags->c = 1;
      break;
    case 'h':
      flags->h = 1;
      break;
    case 'i':
      flags->i = 1;
      break;
    case 'l':
      flags->l = 1;
      break;
    case 'n':
      flags->n = 1;
      break;
    case 'o':
      flags->o = 1;
      break;
    case 's':
      flags->s = 1;
      break;
    case 'v':
      flags->v = 1;
      break;
  }
}
//======================================================================================
//======================================================================================
int extractMatchingLines(FILE* file, Flags* flags, LinesList* result,
                         int patternCount, regex_t regexes[patternCount]) {
  int err_code = G_SUCCESS;
  char* line = NULL;
  size_t linesize = 0;
  ssize_t linelen = 0;
  size_t line_counter = 0;
  while (err_code == G_SUCCESS && (!flags->l || result->count == 0) &&
         (linelen = getline(&line, &linesize, file)) != -1) {
    ++line_counter;
    /* Quitar '\n' final */
    if (linelen > 0 && line[linelen - 1] == '\n') {
      line[linelen - 1] = '\0';
    }
    int reg_result = REG_NOMATCH;
    for (int i = 0;
         err_code == G_SUCCESS && i < patternCount && reg_result != 0; i++) {
      reg_result = regexec(&regexes[i], line, 0, NULL, 0);
      if (reg_result != 0 && reg_result != REG_NOMATCH) {
        err_code = G_USAGE_ERROR;
        fprintf(stderr, "exec: could not exec regex\n");
      }
    }

    if (err_code == G_SUCCESS) {
      if (flags->v) {
        reg_result = (reg_result == 0 ? REG_NOMATCH : 0);
      }
      if (reg_result == 0) {
        err_code = addLineToLineList(result, line, line_counter);
      }
    }
  }

  if (linelen == -1 && !feof(file)) {
    err_code = G_USAGE_ERROR;
    perror("getline");
  }
  free(line);
  return err_code;
}
//======================================================================================
int prepareRegex(LinesList* patterns, regex_t* regexes, Flags* flags) {
  int err_code = G_SUCCESS;
  int regcomp_flags = REG_EXTENDED;
  if (flags->i) regcomp_flags |= REG_ICASE;

  for (size_t i = 0; err_code == G_SUCCESS && i < patterns->count; i++) {
    if (regcomp(&regexes[i], patterns->lines[i].value, regcomp_flags) != 0) {
      fprintf(stderr, "regcomp: could not compile regex \"%s\"\n",
              patterns->lines[i].value);
      err_code = G_USAGE_ERROR;
    }
  }

  return err_code;
}
//======================================================================================
int addLineToLineList(LinesList* linelist, char* line, size_t lineNumber) {
  int result = G_SUCCESS;
  linelist->lines = realloc(linelist->lines,
                            sizeof(*linelist->lines) * (linelist->count + 1));

  if (linelist->lines == NULL) {
    result = G_USAGE_ERROR;
    perror("realloc");
  } else {
    linelist->lines[linelist->count].value =
        (char*)calloc(strlen(line) + 1, sizeof(char));
    if (linelist->lines[linelist->count].value == NULL) {
      result = G_USAGE_ERROR;
      perror("malloc");
    } else {
      strcpy(linelist->lines[linelist->count].value, line);
      linelist->lines[linelist->count].number = lineNumber;
      linelist->count++;
    }
  }
  return result;
}
//======================================================================================
void freeLinesList(LinesList* linelist) {
  for (size_t i = 0; i < linelist->count; i++) free(linelist->lines[i].value);
  free(linelist->lines);
}
//======================================================================================
void printUsage(const char* programName) {
  fprintf(stderr,
          "Usage: %s [-e pattern] [-f file] [-i] [-v] [-c] "
          "[-l] [-n] [-h] [-s] [-o] [pattern] [file...]\n",
          programName);
}
//======================================================================================
void printSelectedLines(Flags* flags, LinesList* lines, const char* filepath) {
  for (size_t i = 0; i < lines->count; i++) {
    if (filepath && !flags->h) printf("%s:", filepath);
    if (flags->n) printf("%zu:", lines->lines[i].number);

    printf("%s\n", lines->lines[i].value);
  }
}
//======================================================================================
// THIS IS FOR -F
int readPatternFromFile(LinesList* patterns, const char* filename) {
  int result = G_SUCCESS;
  FILE* pat_file = fopen(filename, "r");
  if (pat_file == NULL) {
    result = G_USAGE_ERROR;
    perror(filename);
  } else {
    ssize_t linelen;
    char* line = NULL;
    size_t linesize = 0;
    while (result == G_SUCCESS &&
           (linelen = getline(&line, &linesize, pat_file)) != -1) {
      if (linelen > 0 && line[linelen - 1] == '\n') {
        line[linelen - 1] = '\0';
      }
      result = addLineToLineList(patterns, line, 0);
    }
    if (linelen == -1 && !feof(pat_file)) {
      result = G_USAGE_ERROR;
      perror("getline");
    }
    free(line);
    fclose(pat_file);
  }
  return result;
}
//======================================================================================
// FOR -o
//======================================================================================
int extractMatches(Line* line, LinesList* matches, int patternCount,
                   regex_t regexes[patternCount]) {
  int result = G_SUCCESS;
  char* scanPtr = line->value;
  int matchFound = 0;  // 0 == match OK, REG_NOMATCH==1 no hay match
  while (result == G_SUCCESS && matchFound == 0) {
    matchFound = REG_NOMATCH;
    int regexFlags = (scanPtr == line->value) ? 0 : REG_NOTBOL;
    regmatch_t bestMatch = {.rm_so = (int)strlen(scanPtr) + 1, .rm_eo = -1};
    for (int i = 0; result == G_SUCCESS && i < patternCount; i++) {
      regmatch_t candidateMatch;
      int rc = regexec(&regexes[i], scanPtr, 1, &candidateMatch, regexFlags);
      if (rc != 0 && rc != REG_NOMATCH) {
        result = G_USAGE_ERROR;
        fprintf(stderr, "exec: could not exec regex\n");
      } else if (rc == 0 && candidateMatch.rm_eo > candidateMatch.rm_so) {
        if (compareLeftMostLongestMatches(&bestMatch, &candidateMatch)) {
          bestMatch = candidateMatch;
          matchFound = 0;
        }
      }
    }
    if (result == G_SUCCESS && matchFound == 0) {
      int substrLen = bestMatch.rm_eo - bestMatch.rm_so;
      char buf[substrLen + 1];
      strncpy(buf, scanPtr + bestMatch.rm_so, substrLen);
      buf[substrLen] = '\0';
      result = addLineToLineList(matches, buf, line->number);
      scanPtr += bestMatch.rm_eo;
    }
  }
  return result;
}
//======================================================================================
int compareLeftMostLongestMatches(regmatch_t* currentBest,
                                  regmatch_t* candidate) {
  int is_better = 0;
  if (candidate->rm_so < currentBest->rm_so) {
    is_better = 1;
  } else if (candidate->rm_so == currentBest->rm_so &&
             candidate->rm_eo > currentBest->rm_eo) {
    is_better = 1;
  }

  return is_better;
}
//======================================================================================