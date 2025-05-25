#!/bin/bash

# -Wno-free-nonheap-object	= Desactiva advertencia de free() al compilar
# -lft_malloc				= -l busca lib + ft_malloc + .so
# -Wl,-rpath=./build/lib	= Pasa al linker el parametro rpath para que busque en esa ruta las bibliotecas en runtime

gcc -g -Wno-free-nonheap-object -o testing testing.c -I./inc -L./build/lib -lft_malloc -Wl,-rpath=./build/lib -pthread
./testing
