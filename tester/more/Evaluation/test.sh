#!/bin/sh
@read -p "Next -> lib" ; clear
nm libft_malloc.so | grep --color -E "T _show_alloc_mem|T _realloc|T _free|T _malloc|$$" -

@read -p "Next -> test0 (reference)" ; clear
cat tests/test0.c
gcc -o test0 tests/test0.c
/usr/bin/time -l ./test0 &> /dev/stdout | grep --color -E "reclaims|$$" -

@read -p "Next -> test1 (malloc)" ; clear
cat tests/test1.c
gcc -o test1 tests/test1.c
./run.sh /usr/bin/time -l ./test0 &> /dev/stdout | grep --color reclaims
./run.sh /usr/bin/time -l ./test1 &> /dev/stdout | grep --color reclaims

@read -p "Next -> test2 (free)" ; clear
cat tests/test2.c
gcc -o test2 tests/test2.c
./run.sh /usr/bin/time -l ./test1 &> /dev/stdout | grep --color reclaims
./run.sh /usr/bin/time -l ./test2 &> /dev/stdout | grep --color reclaims

@read -p "Next -> test3 (realloc)" ; clear
cat tests/test3.c
gcc -o test3 tests/test3.c
./run.sh ./test3
@read -p "Next -> test3bis (realloc++)" ; clear
cat tests/test3bis.c
gcc -o test3bis tests/test3bis.c
./run.sh ./test3bis

@read -p "Next -> test4 (error management)" ; clear
cat tests/test4.c
gcc -o test4 tests/test4.c
./run.sh ./test4

@read -p "Next -> test4 (show_alloc_mem)" ; clear
cat tests/test5.c
gcc -o test5 tests/test5.c -L. -lft_malloc