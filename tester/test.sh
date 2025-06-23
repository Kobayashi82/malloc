#!/bin/bash

GREEN="\033[0;32m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
RED="\033[0;31m"
NC="\033[0m"

TESTER_DIR="$BASE_DIR"
LIB_DIR="../build/lib"

if [ -f ".gitignore" ]; then
	make; cd tester
else
	cd ..; make; cd tester
fi

if [ ! -f "$LIB_DIR/libft_malloc.so" ]; then
	echo -e "${RED}No se pudo encontrar la librería ${YELLOW}libft_malloc.so${NC}\n"
	exit 1
fi

normal_test() {
	echo -e " ${GREEN}===============================================${NC}"
	echo -e "${YELLOW}                 Malloc Tester                 ${NC}"
	echo -e " ${GREEN}===============================================${NC}"
	clang -g -o test aprintf.c test.c
	(
		export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
		export LD_PRELOAD="libft_malloc.so"
		./test
	)
}

debug_test() {
	echo -e " ${GREEN}===============================================${NC}"
	echo -e "${YELLOW}             Malloc Tester (DEBUG)             ${NC}"
	echo -e " ${GREEN}===============================================${NC}"
	clang -g -o test aprintf.c test.c -DDEBUG_MODE
	(
		export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
		export LD_PRELOAD="libft_malloc.so"
		./test
	)
}

leaks_test() {
	echo -e "${RED}valgrind fuerza el uso del allocator native${NC}\n"
	exit 1
	if ! command -v valgrind &> /dev/null; then
		echo -e "${RED}valgrind no está instalado${NC}\n"
		exit 1
	fi
	echo -e " ${GREEN}===============================================${NC}"
	echo -e "${YELLOW}             Malloc Tester (LEAKS)             ${NC}"
	echo -e " ${GREEN}===============================================${NC}\n"
	clang -g -o test aprintf.c test.c
	(
		export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
		export LD_PRELOAD="libft_malloc.so"
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./test
	)
	echo
}

basic_test() {
	echo -e " ${GREEN}===============================================${NC}"
	echo -e "${YELLOW}              Malloc Tester (BASIC)            ${NC}"
	echo -e " ${GREEN}===============================================${NC}\n"
	clang -g -Wno-free-nonheap-object -o test aprintf.c basic.c -I../inc -L${LIB_DIR} -lft_malloc -Wl,-rpath=${LIB_DIR} -pthread
	./test
	echo
}

case "$1" in
	"debug")					debug_test;;
	"leak"|"leaks"|"valgrind")	leaks_test;;
	"basic")					basic_test;;
	*)							normal_test;;
esac

rm test

# -Wno-free-nonheap-object	= Desactiva advertencia de free() al compilar
# -lft_malloc				= -l busca lib + ft_malloc + .so
# -Wl,-rpath=./build/lib	= Pasa al linker el parametro rpath para que busque en esa ruta las bibliotecas en runtime

# Terminal:
# export LD_LIBRARY_PATH="../build/lib:$LD_LIBRARY_PATH"
# export LD_PRELOAD="libft_malloc.so"
#
# export LD_LIBRARY_PATH="/home/vzurera-/42/malloc/build/lib:$LD_LIBRARY_PATH" && export LD_PRELOAD="libft_malloc.so"

# export LD_LIBRARY_PATH="/mnt/d/Programas/Mis Programas/42/Active/malloc/build/lib:$LD_LIBRARY_PATH" && export LD_PRELOAD="libft_malloc.so"

# export LD_LIBRARY_PATH="/home/kobay/malloc/build/lib:$LD_LIBRARY_PATH" && export LD_PRELOAD="libft_malloc.so"

# export MALLOC_DEBUG=1
#
# ./test

# gdb:
# set environment LD_LIBRARY_PATH="../build/lib:$LD_LIBRARY_PATH"
# set environment LD_PRELOAD="libft_malloc.so"
