#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

# Function to print colored output
print_header() {
    echo -e "${BLUE}================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${CYAN} $1${NC}"
}

cd ..
if ! make; then	exit 1; fi

if [ -f "./tester/load.sh" ]; then
    source ./tester/load.sh
	export MALLOC_CHECK_=1
	export MALLOC_DEBUG=1
else
    exit 1
fi

cd tester/complete
if make -s all_silent; then
	echo ""
    print_success "All test suites compiled successfully"
else
	echo ""
    print_error "Failed to compile test suites"
    exit 1
fi

# Check if user wants to run specific tests
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo -e "${CYAN}Usage: $0 [options]${NC}"
    echo ""
    echo "Options:"
    echo "  --help, -h       Show this help message"
    echo "  --main           Run only main functions test"
    echo "  --alignment      Run only alignment functions test"
    echo "  --extra          Run only extra functions test"
    echo "  --stress         Run only stress test"
    echo "  --quick          Run all tests without detailed output"
    echo "  (no options)     Run all tests with detailed output"
    exit 0
elif [ "$1" = "--main" ]; then
    echo ""
    ./test_main
elif [ "$1" = "--alignment" ]; then
    echo ""
    ./test_alignment
elif [ "$1" = "--extra" ]; then
    echo ""
    ./test_extra
elif [ "$1" = "--stress" ]; then
    echo ""
    ./test_stress
elif [ "$1" = "--quick" ]; then
    echo ""
    ./run_all_tests
else
    echo ""
    ./run_all_tests
fi

test_result=$?

# Step 5: Summary
echo -e "${CYAN}========================================${NC}"

rm -f run_all_tests test_main test_extra test_alignment test_stress 2> /dev/null

source ../unload.sh
unset MALLOC_CHECK_
unset MALLOC_DEBUG

exit $test_result

# -Wno-free-nonheap-object	= Desactiva advertencia de free() al compilar
# -lft_malloc				= -l busca lib + ft_malloc + .so
# -Wl,-rpath=./lib	= Pasa al linker el parametro rpath para que busque en esa ruta las bibliotecas en runtime

# Unix:
# export LD_LIBRARY_PATH="/home/kobay/malloc/lib:$LD_LIBRARY_PATH" && export LD_PRELOAD="libft_malloc.so"
# export MALLOC_DEBUG=1

# Mac:
# export DYLD_LIBRARY_PATH="/[project_path]/lib:$DYLD_LIBRARY_PATH" && export DYLD_INSERT_LIBRARIES="/[project_path]/lib/libft_malloc.so"
