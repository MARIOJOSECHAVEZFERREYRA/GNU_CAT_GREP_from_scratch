#!/bin/bash

SUCCESS=0
FAIL=0
COUNTER=0
RESULT=0
DIFF_RES=""

declare -a tests=(
"for s21_grep.c s21_grep.h Makefile VAR"
"for s21_grep.c VAR"
"-e for -e ^int s21_grep.c s21_grep.h Makefile VAR"
"-e for -e ^int s21_grep.c VAR"
"-e regex -e ^print s21_grep.c VAR"
"-e while -e void s21_grep.c Makefile VAR"
)

testing()
{
    t=$(echo $@ | sed "s/VAR/$var/")

    # Ejecutar valgrind con las opciones correctas
    valgrind --leak-check=full --track-origins=yes --quiet ./s21_grep $t > test_s21_grep.log
    leak=$(grep -A100000 "leak" test_s21_grep.log)

    if [[ $leak == *"0 leaks for 0 total leaked bytes"* ]]
    then
        (( SUCCESS++ ))
        echo -e "\033[31m$FAIL\033[0m/\033[32m$SUCCESS\033[0m/$COUNTER \033[32msuccess\033[0m grep $t"
    else
        (( FAIL++ ))
        echo -e "\033[31m$FAIL\033[0m/\033[32m$SUCCESS\033[0m/$COUNTER \033[31mfail\033[0m grep $t"
        # Mostrar las fugas para debugging
        echo "$leak"
    fi
    rm -f test_s21_grep.log
}

# 1 parámetro
for var1 in v c l n h o
do
    for i in "${tests[@]}"
    do
        (( COUNTER++ ))
        var="-$var1"
        testing $i
    done
done

# 2 parámetros
for var1 in v c l n h o
do
    for var2 in v c l n h o
    do
        if [ "$var1" != "$var2" ]
        then
            for i in "${tests[@]}"
            do
                (( COUNTER++ ))
                var="-$var1 -$var2"
                testing $i
            done
        fi
    done
done

# 3 parámetros
for var1 in v c l n h o
do
    for var2 in v c l n h o
    do
        for var3 in v c l n h o
        do
            if [ "$var1" != "$var2" ] && [ "$var2" != "$var3" ] && [ "$var1" != "$var3" ]
            then
                for i in "${tests[@]}"
                do
                    (( COUNTER++ ))
                    var="-$var1 -$var2 -$var3"
                    testing $i
                done
            fi
        done
    done
done

# 2 parámetros duplicados
for var1 in v c l n h o
do
    for var2 in v c l n h o
    do
        if [ "$var1" != "$var2" ]
        then
            for i in "${tests[@]}"
            do
                (( COUNTER++ ))
                var="-$var1$var2"
                testing $i
            done
        fi
    done
done

# 3 parámetros combinados
for var1 in v c l n h o
do
    for var2 in v c l n h o
    do
        for var3 in v c l n h o
        do
            if [ "$var1" != "$var2" ] && [ "$var2" != "$var3" ] && [ "$var1" != "$var3" ]
            then
                for i in "${tests[@]}"
                do
                    (( COUNTER++ ))
                    var="-$var1$var2$var3"
                    testing $i
                done
            fi
        done
    done
done

echo -e "\033[31mFAIL: $FAIL\033[0m"
echo -e "\033[32mSUCCESS: $SUCCESS\033[0m"
echo "ALL: $COUNTER"
