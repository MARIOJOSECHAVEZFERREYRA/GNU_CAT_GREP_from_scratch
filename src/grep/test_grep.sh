#!/usr/bin/env bash

SUCCESS=0
FAIL=0
COUNTER=1
ERROR_LOG="error.log"
TMP1="tmp_s21.txt"
TMP2="tmp_sys.txt"

: > "$ERROR_LOG"

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

testing() {
    local idx="$1"; shift
    local args="$*"
    local cmd="grep $args"

    printf "test%3d: " "$idx"

    grep $args > "$TMP2" 2>/dev/null
    local code_grep=$?

    if [ $code_grep -eq 2 ]; then
        ./s21_grep $args > /dev/null 2>/dev/null
        local code_s21=$?
        if [ $code_s21 -eq 2 ]; then
            echo "PASSED"
            (( SUCCESS++ ))
        else
            echo "FAILED (usage error) - $cmd"
            (( FAIL++ ))
            {
                echo "----- TEST $idx FAILED (usage error) -----"
                echo "Command: $cmd"
                echo "grep exit code: $code_grep"
                echo "s21_grep exit code: $code_s21"
                echo
            } >> "$ERROR_LOG"
        fi
        return
    fi

    ./s21_grep $args > "$TMP1" 2>/dev/null
    local code_s21=$?

    if [ $code_s21 -eq $code_grep ] && diff -q "$TMP1" "$TMP2" &>/dev/null; then
        echo "PASSED"
        (( SUCCESS++ ))
    else
        echo "FAILED - $cmd"
        (( FAIL++ ))
        {
            echo "----- TEST $idx FAILED -----"
            echo "Command: $cmd"
            echo "grep      exit code: $code_grep"
            echo "s21_grep  exit code: $code_s21"
            echo "Diff stdout:"
            diff "$TMP2" "$TMP1" | sed 's/^/    /'
            echo
        } >> "$ERROR_LOG"
    fi

    rm -f "$TMP1" "$TMP2"
}

flags=( e i v c l n h s o f )
n=${#flags[@]}

for ((i_flag = 0; i_flag < n; i_flag++)); do
  for template in "${multy_testing[@]}"; do
    var="-${flags[i_flag]}"
    eval "cmd_line=\"${template//VAR/\$var}\""
    testing "$COUNTER" "$cmd_line"
    (( COUNTER++ ))
  done
done

for ((i_flag = 0; i_flag < n - 1; i_flag++)); do
  for ((j_flag = i_flag + 1; j_flag < n; j_flag++)); do
    for template in "${multy_testing[@]}"; do
      eval "replacement=\"-${flags[i_flag]} -${flags[j_flag]}\""
      eval "cmd_line=\"${template//VAR/\$replacement}\""
      testing "$COUNTER" "$cmd_line"
      (( COUNTER++ ))
    done
  done
done

for template in "${unique_testing[@]}"; do
    testing "$COUNTER" "$template"
    (( COUNTER++ ))
done

echo
echo "========================================="
echo " Total PASSED: $SUCCESS"
echo " Total FAILED: $FAIL"
echo " Total TESTS:  $((COUNTER-1))"
echo "========================================="
echo "More information in $ERROR_LOG"