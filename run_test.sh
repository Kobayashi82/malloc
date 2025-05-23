#!/bin/bash

# Colores
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
RED="\033[0;31m"
NC="\033[0m"

# Directorio base del proyecto
BASE_DIR="$(cd "$(dirname "$0")" && pwd)"
TESTER_DIR="$BASE_DIR/src/tester"
LIB_DIR="$BASE_DIR/build/lib"

# Asegurarse de que la librería está compilada primero
echo -e "${CYAN}Compilando librería malloc...${NC}"
cd "$BASE_DIR"
make

# Comprobar si la compilación tuvo éxito
if [ ! -f "$LIB_DIR/libft_malloc.so" ]; then
    echo -e "${RED}Error: No se pudo encontrar la librería compilada.${NC}"
    exit 1
fi

# Compilar el tester
echo -e "${CYAN}Compilando tester...${NC}"
cd "$TESTER_DIR"
make clean

# Compilar manualmente para asegurar el enlace correcto
echo -e "${CYAN}Compilando test_malloc con enlace directo a la biblioteca...${NC}"
gcc -Wall -Wextra -g -o test_malloc main.c -I"$BASE_DIR/inc" -L"$LIB_DIR" -lft_malloc -pthread
echo -e "${GREEN}✓${NC} Tester compilado con enlace directo"

# Ejecutar el programa con rutas de bibliotecas
echo -e "${GREEN}===============================================${NC}"
echo -e "${YELLOW}Ejecutando test con malloc personalizado${NC}"
echo -e "${GREEN}===============================================${NC}"

# Configurar LD_LIBRARY_PATH para encontrar nuestra biblioteca
export LD_LIBRARY_PATH="$LIB_DIR:$LD_LIBRARY_PATH"

# Mostrar la ruta real de la biblioteca para depuración
echo -e "${CYAN}Usando biblioteca: ${YELLOW}$LIB_DIR/libft_malloc.so${NC}"
echo -e "${CYAN}LD_LIBRARY_PATH: ${YELLOW}$LD_LIBRARY_PATH${NC}"

# Opción 1: Con variable de entorno para debug
echo -e "${CYAN}Ejecutando sin debug...${NC}"
./test_malloc

# Opción 2: Con debug activado directamente en el programa
echo -e "\n${CYAN}¿Quieres ejecutar con mensajes de debug de depuración? (s/n)${NC}"
read -r response
if [[ "$response" =~ ^([sS]|[sS][iI])$ ]]; then
    echo -e "${GREEN}===============================================${NC}"
    echo -e "${YELLOW}Ejecutando con debug activado${NC}"
    echo -e "${GREEN}===============================================${NC}"
    
    # Recompilamos con la opción de debug activada
    gcc -Wall -Wextra -g -o test_malloc main.c -I"$BASE_DIR/inc" -L"$LIB_DIR" -lft_malloc -pthread -DDEBUG_MODE
    ./test_malloc
fi

# Mostrar uso de memoria con valgrind (opcional)
if command -v valgrind &> /dev/null; then
    echo -e "\n${CYAN}¿Quieres ejecutar el test con valgrind para verificar fugas de memoria? (s/n)${NC}"
    read -r response
    if [[ "$response" =~ ^([sS]|[sS][iI])$ ]]; then
        echo -e "${GREEN}===============================================${NC}"
        echo -e "${YELLOW}Ejecutando con valgrind${NC}"
        echo -e "${GREEN}===============================================${NC}"
        valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./test_malloc
    fi
fi

echo -e "${GREEN}Test completado.${NC}"
