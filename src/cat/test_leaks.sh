#!/bin/bash

LEAK_COUNTER=0
COUNTER=0

# Estos son ejemplos de combinaciones para cat; ajusta según tus archivos de prueba
declare -a multy_testing=(
  "VAR"                                        # stdin
  "VAR test1.txt"                              # un archivo
  "test127.txt VAR"                            # stdin al final
  "-b VAR test1.txt test2.txt"                 # varias flags y archivos
  "-s VAR test1.txt"                           # squeeze-blank
  "-benstv VAR test1.txt"                      # flags compuestas
)

declare -a unique_testing=(
  "nofile.txt"                                 # archivo que no existe
  "-n nofile.txt"                              # número de línea en no existente
  "-T test1.txt"                               # mostrar tabs
  "-E test2.txt"                               # mostrar ends
)

vg_checking () {
  # sustituye VAR por el valor de $var
  t=$(echo "$@" | sed "s/VAR/$var/")
  (( COUNTER++ ))
  # En macOS:
  leaks -atExit -- ./s21_cat $t > vg_out.log 2>&1
  VG_RES="$(grep -c "leaked bytes" vg_out.log)"
  # En Linux, usarías en su lugar algo como:
  # valgrind --leak-check=full --log-file=vg_info.log ./s21_cat $t >/dev/null 2>&1
  # VG_RES="$(grep -c "definitely lost:" vg_info.log)"

  if [ "$VG_RES" -eq 0 ]; then
    echo "$COUNTER: NO LEAKS — ./s21_cat $t"
  else
    echo "$COUNTER: LEAKS DETECTED — ./s21_cat $t"
    (( LEAK_COUNTER++ ))
  fi

  rm -f vg_out.log
  # rm -f vg_info.log  # si usas valgrind en Linux
}

# Tests combinando una sola flag
for f in b e n s t v E T; do
  var="-$f"
  for i in "${multy_testing[@]}"; do
    vg_checking $i
  done
done

# Tests combinando dos flags
for f1 in b e n s t v E T; do
 for f2 in b e n s t v E T; do
  if [ "$f1" != "$f2" ]; then
    var="-$f1$ f2"
    for i in "${multy_testing[@]}"; do
      vg_checking $i
    done
  fi
 done
done

# Algunos casos “únicos” (errores de archivo, flags largas, etc.)
for i in "${unique_testing[@]}"; do
  var="-"
  vg_checking $i
done

echo
echo "LEAKS FOUND: $LEAK_COUNTER of $COUNTER tests."
