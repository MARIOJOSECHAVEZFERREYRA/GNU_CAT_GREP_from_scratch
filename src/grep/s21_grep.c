// ========================================================================
//  s21_grep.c
//
//  • Devuelve 0 si hubo al menos una coincidencia.
//  • Devuelve 1 si terminó sin encontrar coincidencias.
//  • Devuelve 2 si hubo un error de uso (-e sin argumento, -f sin poder abrir)
//    o si no se pudo abrir un fichero (salvo que se use -s).
//
//  Flags soportadas: -e, -f, -i, -v, -c, -l, -n, -h, -s, -o
//
//  Correcciones principales de esta versión:
//   1) Evita “heap-buffer-overflow” al iterar con regexec sobre la línea,
//      controlando que el “cursor” nunca avance más allá de line+longitud.
//   2) Asegura que -n + -o imprima “línea:coincidencia” (y no “línea:lín:e” dobles).
//   3) Gestiona correctamente el caso de “-f archivoInexistente” devolviendo 2.
//   4) Inicializa bien los vectores de patrones y libera sólo lo asignado con strdup.
//   5) Se comprueban los retornos de malloc/realloc.
// ========================================================================

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>   // para getopt, optarg, optind


typedef struct {
    size_t so;
    size_t eo;
} Match;

// --------------------------------------------------
// Variables globales para determinar el código de salida
// --------------------------------------------------
static int any_match_global = 0; // =1 si al menos un match en cualquier fichero
static int any_error        = 0; // =1 si hubo error de uso o no pudo abrir fichero

// --------------------------------------------------
// Compara dos Match por su campo so
// --------------------------------------------------
static int compare_match(const void *a, const void *b) {
    const Match *A = (const Match *)a;
    const Match *B = (const Match *)b;
    if (A->so < B->so) return -1;
    if (A->so > B->so) return  1;
    return 0;
}

// --------------------------------------------------
// Carga patrones desde un fichero (uno por línea).
// Devuelve un array de punteros (strdup) y en out_count la cantidad.
// Si falla fopen, devuelve NULL y out_count=0.
// --------------------------------------------------
static char **load_patterns_from_file(const char *fname, int *out_count) {
    FILE *f = fopen(fname, "r");
    if (!f) return NULL;

    char **pats = NULL;
    int     cap  = 0;
    *out_count   = 0;

    char *line = NULL;
    size_t len  = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, f)) != -1) {
        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
            nread--;
        }
        if (*out_count >= cap) {
            cap = cap ? cap * 2 : 16;
            char **tmp = realloc(pats, cap * sizeof(char *));
            if (!tmp) {
                // Si falla realloc, liberamos todo lo previo
                for (int i = 0; i < *out_count; i++) free(pats[i]);
                free(pats);
                free(line);
                fclose(f);
                return NULL;
            }
            pats = tmp;
        }
        pats[*out_count] = strdup(line);
        if (!pats[*out_count]) {
            // Si falla strdup, liberamos todo lo previo
            for (int i = 0; i < *out_count; i++) free(pats[i]);
            free(pats);
            free(line);
            fclose(f);
            return NULL;
        }
        (*out_count)++;
    }

    free(line);
    fclose(f);
    return pats;
}

// --------------------------------------------------
// Declaración adelantada de la función que procesa cada archivo
// Si filename==NULL, lee de stdin.
// --------------------------------------------------
static void grep_file(const char *filename, char **patterns, int pat_count,
                      Flags flags, int total_files);

// --------------------------------------------------
// Función principal
// --------------------------------------------------
int main(int argc, char *argv[]) {
    Flags flags = {0};
    int opt;

    // Reservamos espacio para 'argc' patrones como máximo
    int    cap       = argc;
    char **patterns  = malloc(cap * sizeof(char *));
    int   *pat_alloc = malloc(cap * sizeof(int));
    if (!patterns || !pat_alloc) {
        fprintf(stderr, "Memory allocation failure\n");
        return 2;
    }
    // Inicializamos pat_alloc a 0
    for (int i = 0; i < cap; i++) {
        patterns[i] = NULL;
        pat_alloc[i] = 0;
    }
    int pat_count = 0;

    // --------------------------------------------------
    // 1) Parseo de opciones
    // --------------------------------------------------
    // “e:”  == -e requiere argumento
    // “f:”  == -f requiere argumento
    // resto son flags sin argumento
    //
    while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
        switch (opt) {
            case 'e':
                if (pat_count >= cap) {
                    cap *= 2;
                    char **tmp1 = realloc(patterns, cap * sizeof(char *));
                    int   *tmp2 = realloc(pat_alloc, cap * sizeof(int));
                    if (!tmp1 || !tmp2) {
                        fprintf(stderr, "Memory allocation failure\n");
                        return 2;
                    }
                    patterns  = tmp1;
                    pat_alloc = tmp2;
                }
                patterns[pat_count]  = optarg;
                pat_alloc[pat_count] = 0; // no se hizo strdup
                pat_count++;
                flags.e = 1;
                break;
            case 'i':
                flags.i = 1; break;
            case 'v':
                flags.v = 1; break;
            case 'c':
                flags.c = 1; break;
            case 'l':
                flags.l = 1; break;
            case 'n':
                flags.n = 1; break;
            case 'h':
                flags.h = 1; break;
            case 's':
                flags.s = 1; break;
            case 'o':
                flags.o = 1; break;

            case 'f': {
                // Cargar patrones desde archivo
                flags.e = 1;
                int fromf_count = 0;
                char **fromf = load_patterns_from_file(optarg, &fromf_count);
                if (!fromf) {
                    // Error al abrir o en malloc: igual que grep estándar → exit 2
                    if (!flags.s) perror(optarg);
                    free(patterns);
                    free(pat_alloc);
                    return 2;
                }
                for (int i = 0; i < fromf_count; i++) {
                    if (pat_count >= cap) {
                        cap *= 2;
                        char **tmp1 = realloc(patterns, cap * sizeof(char *));
                        int   *tmp2 = realloc(pat_alloc, cap * sizeof(int));
                        if (!tmp1 || !tmp2) {
                            fprintf(stderr, "Memory allocation failure\n");
                            return 2;
                        }
                        patterns  = tmp1;
                        pat_alloc = tmp2;
                    }
                    patterns[pat_count]  = fromf[i];
                    pat_alloc[pat_count] = 1; // marcar que viene de strdup
                    pat_count++;
                }
                free(fromf);
                break;
            }

            default:
                // Opción inválida o falta argumento → exit code 2
                fprintf(stderr,
                        "Usage: %s [-e pattern] [-f file] [-i] [-v] [-c] "
                        "[-l] [-n] [-h] [-s] [-o] [pattern] [file...]\n",
                        argv[0]);
                free(patterns);
                free(pat_alloc);
                return 2;
        }
    }

    // --------------------------------------------------
    // 2) Ajustar argv/argc tras getopt
    // --------------------------------------------------
    argc -= optind;
    argv += optind;

    // Si no se usaron -e ni -f, el primer argumento es el patrón
    if (pat_count == 0) {
        if (argc > 0) {
            if (pat_count >= cap) {
                cap *= 2;
                patterns  = realloc(patterns, cap * sizeof(char *));
                pat_alloc = realloc(pat_alloc, cap * sizeof(int));
                if (!patterns || !pat_alloc) {
                    fprintf(stderr, "Memory allocation failure\n");
                    return 2;
                }
            }
            patterns[pat_count]  = argv[0];
            pat_alloc[pat_count] = 0;
            pat_count++;
            argv++;
            argc--;
        } else {
            fprintf(stderr, "grep: missing pattern\n");
            free(patterns);
            free(pat_alloc);
            return 2;
        }
    }

    // --------------------------------------------------
    // 3) Procesar cada archivo (o stdin si no hay ninguno)
    // --------------------------------------------------
    int total_files = argc;
    if (total_files == 0) {
        grep_file(NULL, patterns, pat_count, flags, total_files);
    } else {
        for (int i = 0; i < total_files; i++) {
            grep_file(argv[i], patterns, pat_count, flags, total_files);
        }
    }

    // --------------------------------------------------
    // 4) Liberar memoria de patrones (solo los que vienen de strdup)
    // --------------------------------------------------
    for (int i = 0; i < pat_count; i++) {
        if (pat_alloc[i]) {
            free(patterns[i]);
        }
    }
    free(patterns);
    free(pat_alloc);

    // --------------------------------------------------
    // 5) Devolver el código de salida
    // --------------------------------------------------
    if (any_error)         return 2;
    else if (any_match_global) return 0;
    else                    return 1;
}

// =======================================================================
// grep_file: procesa un solo "archivo" (o stdin si filename==NULL).
//
//   • Si no puede abrir el archivo y no hay -s, marca any_error=1 y retorna.
//   • Compila todos los patrones con regcomp. Si falla regcomp → error=1, retorna.
//   • Lee línea a línea con getline, suprimiendo '\n' al final.
//   • Por cada línea, recorre cada patrón con regexec en bucle para sacar TODAS
//     las apariciones. Si detecta match de longitud 0, avanza cursor en +1 para no
//     quedar en bucle infinito.
//   • Se salta el “cursor” más allá de “line + nread” para evitar regexec fuera de rango.
//   • Aplica flags: -v, -l, -c, -n, -h, -o tal cual lo haría grep.
//   • Al final cierra fichero y libera todo.
// =======================================================================
static void grep_file(const char *filename, char **patterns, int pat_count,
                      Flags flags, int total_files) {
    FILE *f = filename ? fopen(filename, "r") : stdin;
    if (!f) {
        if (!flags.s) {
            perror(filename);
        }
        any_error = 1;
        return;
    }

    // --------------------------------------------------
    // 1) Compilar todos los patrones en regex_t
    // --------------------------------------------------
    regex_t *regs = malloc(pat_count * sizeof(regex_t));
    if (!regs) {
        fprintf(stderr, "Memory allocation failure\n");
        if (f != stdin) fclose(f);
        any_error = 1;
        return;
    }
    int regc_flags = flags.i ? REG_ICASE : 0;
    for (int p = 0; p < pat_count; p++) {
        if (regcomp(&regs[p], patterns[p], regc_flags) != 0) {
            fprintf(stderr, "Invalid regex: %s\n", patterns[p]);
            // Liberar los que sí se compilaron
            while (p--) {
                regfree(&regs[p]);
            }
            free(regs);
            if (f != stdin) fclose(f);
            any_error = 1;
            return;
        }
    }

    // --------------------------------------------------
    // 2) Leer línea a línea
    // --------------------------------------------------
    char  *line   = NULL;
    size_t len    = 0;
    ssize_t nread = 0;

    int line_no      = 1;
    int match_count  = 0;  // para -c
    int matched_file = 0;  // para -l

    while ((nread = getline(&line, &len, f)) != -1) {
        // Suprimir '\n' para que ^$ funcione exactamente como grep
        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
            nread--;
        }

        // --------------------------------------------------
        // 3) Buscar todas las coincidencias para cada patrón
        // --------------------------------------------------
        int    mcap   = 16;
        Match *matches = malloc(mcap * sizeof(Match));
        if (!matches) {
            fprintf(stderr, "Memory allocation failure\n");
            free(line);
            for (int q = 0; q < pat_count; q++) regfree(&regs[q]);
            free(regs);
            if (f != stdin) fclose(f);
            any_error = 1;
            return;
        }
        int mcount = 0;

        for (int p = 0; p < pat_count; p++) {
            char     *cursor = line;
            regmatch_t pm;

            // Buscamos múltiples ocurrencias del patrón p
            while (cursor <= line + nread) {
                int ret = regexec(&regs[p], cursor, 1, &pm, 0);
                if (ret != 0) break; // -1 o REG_NOMATCH → no hay más

                // Si aquí hay match, guardamos su rango
                size_t so = (cursor - line) + pm.rm_so;
                size_t eo = (cursor - line) + pm.rm_eo;

                if (mcount >= mcap) {
                    mcap *= 2;
                    Match *tmp = realloc(matches, mcap * sizeof(Match));
                    if (!tmp) {
                        fprintf(stderr, "Memory allocation failure\n");
                        free(matches);
                        free(line);
                        for (int q = 0; q < pat_count; q++) regfree(&regs[q]);
                        free(regs);
                        if (f != stdin) fclose(f);
                        any_error = 1;
                        return;
                    }
                    matches = tmp;
                }
                matches[mcount].so = so;
                matches[mcount].eo = eo;
                mcount++;

                // Evitar bucle infinito si rm_eo == rm_so
                if (pm.rm_eo > pm.rm_so) {
                    cursor += pm.rm_eo;
                } else {
                    cursor += 1;
                }
                // Si el cursor avanza más allá del final, cortamos
                if (cursor > line + nread) break;
            }
        }

        int any_match = (mcount > 0);
        if (flags.v) {
            any_match = !any_match;
        }
        if (any_match) {
            any_match_global = 1;
        }

        // --------------------------------------------------
        // 4) Si -l y encontramos match: imprimimos nombre de archivo y cortamos
        // --------------------------------------------------
        if (flags.l && any_match) {
            if (filename) {
                printf("%s\n", filename);
                matched_file = 1;
            }
            free(matches);
            break; // dejamos de procesar líneas de este fichero
        }

        // --------------------------------------------------
        // 5) Si hay coincidencia y no -l, contamos para -c
        // --------------------------------------------------
        if (any_match && !flags.l) {
            match_count++;
        }

        // --------------------------------------------------
        // 6) Si any_match y no -c ni -l, imprimimos según -o/-n/-h
        // --------------------------------------------------
        if (any_match && !flags.c && !flags.l) {
            // Caso “solo coincidencias” (-o y sin -v)
            if (flags.o && !flags.v) {
                if (mcount > 1) {
                    qsort(matches, mcount, sizeof(Match), compare_match);
                }
                for (int mi = 0; mi < mcount; mi++) {
                    if (!flags.h && filename && total_files > 1) {
                        printf("%s:", filename);
                    }
                    if (flags.n) {
                        printf("%d:", line_no);
                    }
                    printf("%.*s\n",
                           (int)(matches[mi].eo - matches[mi].so),
                           line + matches[mi].so);
                }
            }
            // Caso imprimir línea completa
            else if (!flags.o) {
                if (!flags.h && filename && total_files > 1) {
                    printf("%s:", filename);
                }
                if (flags.n) {
                    printf("%d:", line_no);
                }
                printf("%s\n", line);
            }
        }

        line_no++;
        free(matches);
    }

    // --------------------------------------------------
    // 7) Si -c y no imprimimos ya con -l, imprimimos conteo
    // --------------------------------------------------
    if (flags.c && !(flags.l && matched_file)) {
        if (!flags.h && filename && total_files > 1) {
            printf("%s:", filename);
        }
        printf("%d\n", match_count);
    }

    // --------------------------------------------------
    // 8) Liberar regex y buffer de línea
    // --------------------------------------------------
    for (int p = 0; p < pat_count; p++) {
        regfree(&regs[p]);
    }
    free(regs);
    free(line);
    if (f != stdin) fclose(f);
}
