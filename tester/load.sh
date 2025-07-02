#!/bin/bash

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo -e "\n\033[0;31m This script must be sourced, not executed directly\033[0m\n"
    echo -e "\033[0;36m  Usage: \033[0;33msource $0\033[0m\n"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_LIB_DIR="$SCRIPT_DIR/../build/lib"

if [ -d "$BUILD_LIB_DIR" ]; then
    LIB_PATH="$(cd "$BUILD_LIB_DIR" && pwd)/libft_malloc.so"
else
    LIB_PATH="$(readlink -f "$BUILD_LIB_DIR")/libft_malloc.so"
fi

if [ ! -f "$LIB_PATH" ]; then
    echo -e "\n\033[0;31m Library not found at \033[0;33m$LIB_PATH\033[0m\n"
    return 1 2>/dev/null || exit 1
fi

OS="$(uname -s)"

case "$OS" in
    Linux*)
        export LD_PRELOAD="$LIB_PATH"
        ;;
    Darwin*)
        export DYLD_INSERT_LIBRARIES="$LIB_PATH"
        export DYLD_FORCE_FLAT_NAMESPACE=1
        ;;
    *)
        echo -e "\n\033[0;31m Unsupported operating system: $OS\033[0m\n"
        return 1 2>/dev/null || exit 1
        ;;
esac

echo -e "\n\033[0;32m Library loaded successfully\033[0m\n"

# Calculate relative path to unload.sh from current working directory
CURRENT_DIR="$(pwd)"
UNLOAD_SCRIPT="$SCRIPT_DIR/unload.sh"
RELATIVE_UNLOAD=$(realpath --relative-to="$CURRENT_DIR" "$UNLOAD_SCRIPT" 2>/dev/null || echo "$UNLOAD_SCRIPT")

# Add ./ prefix if the path doesn't start with / or ./
if [[ "$RELATIVE_UNLOAD" != /* ]] && [[ "$RELATIVE_UNLOAD" != ./* ]]; then
    RELATIVE_UNLOAD="./$RELATIVE_UNLOAD"
fi

echo -e "\033[0;36m To unload the library run: \033[0;33msource $RELATIVE_UNLOAD\033[0m\n"
