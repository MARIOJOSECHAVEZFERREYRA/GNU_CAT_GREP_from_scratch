#ifndef S21_GREP_H_
#define S21_GREP_H_

#include <regex.h>
#include <stdio.h>
#include <unistd.h>

#define G_SUCCESS 0      // al menos una coincidencia
#define G_NO_MATCH 1     // sin coincidencias
#define G_USAGE_ERROR 2  // error de uso o fallo al abrir fichero (salvo -s)

typedef struct {
    int e, i, v, c, l, n, h, s, o;
} Flags;

typedef struct {
    char* value;
    size_t number;
} Line;

typedef struct {
    size_t count;
    Line* lines;
} LinesList;

int grepFile(const char* filename, LinesList* patterns, regex_t regexes[],
             Flags flags, int total_files, int* out_match_count);
void markFlags(Flags* flags, int opt);
int parseInput(int argc, char* argv[], Flags* flags, LinesList* patterns);
int readPatternFromFile(LinesList* patterns, const char* filename);
int addLineToLineList(LinesList* linelist, char* line, size_t lineNumber);
void printUsage(const char* programName);
int prepareRegex(LinesList* patterns, regex_t* regexes, Flags* flags);
void freeLinesList(LinesList* linelist);
void printSelectedLines(Flags* flags, LinesList* lines, const char* filepath);
int extractMatchingLines(FILE* file, Flags* flags, LinesList* result,
                         int patternCount, regex_t regexes[patternCount]);
int extractMatches(Line* line, LinesList* result, int patternCount,
                   regex_t regexes[patternCount]);
int compareLeftMostLongestMatches(regmatch_t* res_match_index,
                                  regmatch_t* temp_match_index);

#endif /* S21_GREP_H_ */
