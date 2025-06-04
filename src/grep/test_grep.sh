#!/usr/bin/env bash
#
# test_grep.sh
#  Script para comparar ./s21_grep vs grep.
#  – stdout comparado entre ambos en la mayoría de casos.
#  – Cuando grep devuelve código 2 (error de uso), se considera PASSED
#    solo si s21_grep también devuelve código 2.
#  – stderr redirigido a /dev/null.
#  – Códigos de error y diferencias de stdout se guardan en error.log.
#  – En terminal solamente “PASSED” (en verde) o “FAILED” (en rojo)
#    junto con el comando testeado.
#

SUCCESS=0
FAIL=0
COUNTER=0
ERROR_LOG="error.log"
TMP1="tmp_s21.txt"
TMP2="tmp_sys.txt"

# Limpiar el archivo de log de errores al iniciar
: > "$ERROR_LOG"

# Códigos de color ANSI
GREEN="\e[32m"
RED="\e[31m"
RESET="\e[0m"

# Plantillas que usan “VAR” para insertar la(s) flag(s)
declare -a multy_testing=(
    "VAR s -o test3.txt"
    "VAR s test1.txt"
    "test127.txt s VAR"
    "D VAR test1.txt"
    "S VAR test1.txt test2.txt test1.txt"
    "-e s VAR -o test3.txt"
    "-e ^int VAR test1.txt test2.txt test1.txt"
    "-f VAR pattern_file.txt test3.txt test2.txt test1.txt"
    "-f pattern_file.txt test3.txt test2.txt test6.txt"
)

# Casos individuales sin “VAR”
declare -a unique_testing=(
    ""
    "-e"
    "-f"
    "s test1.txt"
    "abc no_file.txt"
    "abc -f no_file -ivclnhso no_file.txt"
    "-e S -i -nh test2.txt test1.txt test2.txt test1.txt"
    "-e char -v test2.txt test1.txt"
)

#
# Función de prueba:
#   1) Ejecuta grep con los args y captura code_grep.
#   2) Si code_grep == 2, ejecuta s21_grep y comprueba code_s21:
#        – Si code_s21 == 2 → PASSED.
#        – Si code_s21 != 2 → FAILED y registra en error.log.
#   3) Si code_grep != 2, procede a ejecutar s21_grep y comparar exit codes y stdout:
#        – Si coinciden → PASSED.
#        – Si no coinciden → FAILED y registra en error.log.
#   4) Imprime en terminal solo “PASSED” (verde) o “FAILED” (rojo) con el comando.
#
testing() {
    local idx="$1"; shift
    local args="$*"

    # Mostrar en terminal el test número y el comando
    printf "TEST %3d: grep %s → " "$idx" "$args"

    # 1) Ejecutar grep estándar
    grep $args > "$TMP2" 2>/dev/null
    local code_grep=$?

    # 2) Si grep devolvió 2 (error de uso), chequear s21_grep
    if [ $code_grep -eq 2 ]; then
        ./s21_grep $args > /dev/null 2>/dev/null
        local code_s21=$?
        if [ $code_s21 -eq 2 ]; then
            echo -e "${GREEN}PASSED${RESET}"
            (( SUCCESS++ ))
        else
            echo -e "${RED}FAILED${RESET}"
            (( FAIL++ ))
            {
                echo "----- TEST $idx FAILED (usage error) -----"
                echo "Command: grep $args"
                echo "grep exit code: $code_grep"
                echo "s21_grep exit code: $code_s21"
                echo
            } >> "$ERROR_LOG"
        fi
        (( COUNTER++ ))
        return
    fi

    # 3) Si grep no devolvió 2, comparamos normalmente:
    ./s21_grep $args > "$TMP1" 2>/dev/null
    local code_s21=$?

    if [ $code_s21 -eq $code_grep ] && diff -q "$TMP1" "$TMP2" &>/dev/null; then
        echo -e "${GREEN}PASSED${RESET}"
        (( SUCCESS++ ))
    else
        echo -e "${RED}FAILED${RESET}"
        (( FAIL++ ))
        {
            echo "----- TEST $idx FAILED -----"
            echo "Command: grep $args"
            echo "grep      exit code: $code_grep"
            echo "s21_grep  exit code: $code_s21"
            echo "Diff stdout:"
            diff "$TMP2" "$TMP1" | sed 's/^/    /'
            echo
        } >> "$ERROR_LOG"
    fi

    (( COUNTER++ ))
    rm -f "$TMP1" "$TMP2"
}

# -------------------------------------------------------
# 1) Pruebas “multy_testing” con 1, 2 y 3 flags:
# -------------------------------------------------------
flags=( e i v c l n h s o f )
n=${#flags[@]}

# 1.1) Una sola flag
for ((i_flag = 0; i_flag < n; i_flag++)); do
  for template in "${multy_testing[@]}"; do
    var="-${flags[i_flag]}"
    eval "cmd_line=\"${template//VAR/\$var}\""
    testing "$COUNTER" "$cmd_line"
  done
done

# 1.2) Dos flags combinadas
for ((i_flag = 0; i_flag < n - 1; i_flag++)); do
  for ((j_flag = i_flag + 1; j_flag < n; j_flag++)); do
    for template in "${multy_testing[@]}"; do
      eval "replacement=\"-${flags[i_flag]} -${flags[j_flag]}\""
      eval "cmd_line=\"${template//VAR/\$replacement}\""
      testing "$COUNTER" "$cmd_line"
    done
  done
done


# -------------------------------------------------------
# 2) Pruebas “unique_testing” (sin VAR)
# -------------------------------------------------------
for template in "${unique_testing[@]}"; do
    var="-"  # No se usa VAR, pero definimos var para evitar errores de sintaxis
    testing "$COUNTER" "$template"
done

# -------------------------------------------------------
# Resumen final
# -------------------------------------------------------
echo
echo "========================================="
echo " Total PASSED: $SUCCESS"
echo " Total FAILED: $FAIL"
echo " Total TESTS:  $COUNTER"
echo "========================================="
echo "Detalles de fallos en $ERROR_LOG"
