#!/bin/bash

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo -e "\n\033[0;31m This script must be sourced, not executed directly\033[0m\n"
    echo -e "\033[0;36m  Usage: \033[0;33msource $0\033[0m\n"
    exit 1
fi

OS="$(uname -s)"

case "$OS" in
    Linux*)
        if [ -n "$LD_PRELOAD" ]; then
            unset LD_PRELOAD
            echo -e "\n\033[0;32m Library unloaded successfully\033[0m\n"
		else
			echo -e "\n\033[0;31m No library had been loaded\033[0m\n"
        fi
        ;;
    Darwin*)
        if [ -n "$DYLD_INSERT_LIBRARIES" ] || [ -n "$DYLD_FORCE_FLAT_NAMESPACE" ]; then
            unset DYLD_INSERT_LIBRARIES
            unset DYLD_FORCE_FLAT_NAMESPACE
			echo -e "\n\033[0;32m Library unloaded successfully\033[0m\n"
		else
			echo -e "\n\033[0;31m No library had been loaded\033[0m\n"
        fi
        ;;
    *)
        echo -e "\n\033[0;31m Unsupported operating system: $OS\033[0m\n"
        return 1 2>/dev/null || exit 1
        ;;
esac
