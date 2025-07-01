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
	echo -e "${RED}Unable to find library ${YELLOW}libft_malloc.so${NC}\n"
	exit 1
fi

normal_test() {
	echo -e " ${GREEN}===============================================${NC}"
	echo -e "${YELLOW}                 Malloc Tester                 ${NC}"
	echo -e " ${GREEN}===============================================${NC}"
	clang -g -o test tests/koba.c
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
	clang -g -o test tests/koba.c -DDEBUG_MODE
	(
		export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
		export LD_PRELOAD="libft_malloc.so"
		./test
	)
}

leaks_test() {
	if ! command -v valgrind &> /dev/null; then
		echo -e "${RED}valgrind is not installed${NC}\n"
		exit 1
	fi
	echo -e " ${GREEN}===============================================${NC}"
	echo -e "${YELLOW}             Malloc Tester (LEAKS)             ${NC}"
	echo -e " ${GREEN}===============================================${NC}\n"
	clang -g -o test tests/koba.c
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
	clang -g -Wno-free-nonheap-object -o test tests/basic.c -I../inc -L${LIB_DIR} -lft_malloc -Wl,-rpath=${LIB_DIR} -pthread
	./test
	echo
}

complex_test() {
	echo -e " ${GREEN}===============================================${NC}"
	echo -e "${YELLOW}              Malloc Tester (COMPLEX)          ${NC}"
	echo -e " ${GREEN}===============================================${NC}\n"
	clang -g -o test tests/complex.c
	(
		export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
		export LD_PRELOAD="libft_malloc.so"
		./test
	)
	echo
}

realloc_test() {
	echo -e " ${GREEN}===============================================${NC}"
	echo -e "${YELLOW}              Malloc Tester (REALLOC)          ${NC}"
	echo -e " ${GREEN}===============================================${NC}\n"
	clang -g -o test tests/realloc.c
	(
		export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
		export LD_PRELOAD="libft_malloc.so"
		./test
	)
	echo
}

speed_test() {
	echo -e " ${GREEN}===============================================${NC}"
	echo -e "${YELLOW}              Malloc Tester (SPEED)            ${NC}"
	echo -e " ${GREEN}===============================================${NC}\n"
	clang -g -o test tests/speed.c
	(
		export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"
		export LD_PRELOAD="libft_malloc.so"
		./test
	)
	echo
}

case "$1" in
	"debug")					debug_test;;
	"leak"|"leaks"|"valgrind")	leaks_test;;
	"basic")					basic_test;;
	"complex")					complex_test;;
	"realloc")					realloc_test;;
	"speed")					speed_test;;
	*)							normal_test;;
esac

rm test

# -Wno-free-nonheap-object	= Desactiva advertencia de free() al compilar
# -lft_malloc				= -l busca lib + ft_malloc + .so
# -Wl,-rpath=./build/lib	= Pasa al linker el parametro rpath para que busque en esa ruta las bibliotecas en runtime

# Unix:
# export LD_LIBRARY_PATH="/home/kobay/malloc/build/lib:$LD_LIBRARY_PATH" && export LD_PRELOAD="libft_malloc.so"
# export MALLOC_DEBUG=1

# Mac:
# export DYLD_LIBRARY_PATH="/[project_path]/build/lib:$DYLD_LIBRARY_PATH" && export DYLD_INSERT_LIBRARIES="/[project_path]/build/lib/libft_malloc.so"
