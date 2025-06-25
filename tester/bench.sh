#!/bin/bash

# Script to compare performance between native malloc and custom implementation
# Usage: ./malloc_benchmark.sh <program> [arguments...]

echo

if [ $# -eq 0 ]; then
    echo "Usage: $0 <program> [arguments...]"
	echo
    exit 1
fi

PROGRAM="$1"
shift
ARGS="$@"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Verify if the program exists
if ! command -v "$PROGRAM" >/dev/null 2>&1 && [ ! -f "$PROGRAM" ]; then
    echo -e "${RED}Error: Cannot find program '$PROGRAM'${NC}"
	echo
    exit 1
fi

# Temporary files for results
NATIVE_OUTPUT=$(mktemp)
CUSTOM_OUTPUT=$(mktemp)

echo -e "${BLUE}=== Malloc Benchmark ===${NC}"
echo -e "Program: ${YELLOW}$PROGRAM $ARGS${NC}"
echo

# Function to clean temporary files
cleanup() {
    rm -f "$NATIVE_OUTPUT" "$CUSTOM_OUTPUT"
}
trap cleanup EXIT

# Test with native malloc
echo -e "${GREEN}[1/2] Running with native malloc...${NC}"
/usr/bin/time -v "$PROGRAM" $ARGS 2>"$NATIVE_OUTPUT" >/dev/null

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Failed execution with native malloc${NC}"
    exit 1
fi

# Test with custom malloc
echo -e "${GREEN}[2/2] Running with custom malloc...${NC}"

(
    export LD_LIBRARY_PATH="/home/kobay/malloc/build/lib:$LD_LIBRARY_PATH"
    export LD_PRELOAD="libft_malloc.so"
    /usr/bin/time -v "$PROGRAM" $ARGS 2>"$CUSTOM_OUTPUT" >/dev/null
)

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Failed execution with custom malloc${NC}"
    echo -e "${YELLOW}Verify that libft_malloc.so is in the correct path${NC}"
    exit 1
fi

# Función para extraer valor de las métricas
extract_metric() {
    local file="$1"
    local metric="$2"
    local value=$(grep "$metric" "$file" | awk '{print $NF}' | sed 's/[^0-9.]//g')
    if [ -z "$value" ]; then
        echo "0"
    else
        echo "$value"
    fi
}

# Función para extraer tiempo en formato h:mm:ss
extract_time() {
    local file="$1"
    local time_line=$(grep "Elapsed (wall clock) time" "$file" | awk -F': ' '{print $2}')
    if [ -z "$time_line" ]; then
        echo "0"
    else
        echo "$time_line"
    fi
}

# Función para convertir tiempo a segundos
time_to_seconds() {
    local time_str="$1"
    if [ -z "$time_str" ] || [ "$time_str" = "0" ]; then
        echo "0"
        return
    fi
    
    if [[ $time_str =~ ^[0-9]+\.[0-9]+$ ]]; then
        echo "$time_str"
    elif [[ $time_str =~ ^([0-9]+):([0-9]+\.[0-9]+)$ ]]; then
        local minutes=${BASH_REMATCH[1]}
        local seconds=${BASH_REMATCH[2]}
        echo "scale=2; $minutes * 60 + $seconds" | bc -l
    elif [[ $time_str =~ ^([0-9]+):([0-9]+):([0-9]+\.[0-9]+)$ ]]; then
        local hours=${BASH_REMATCH[1]}
        local minutes=${BASH_REMATCH[2]}
        local seconds=${BASH_REMATCH[3]}
        echo "scale=2; $hours * 3600 + $minutes * 60 + $seconds" | bc -l
    else
        echo "0"
    fi
}

# Función para calcular diferencia porcentual
calculate_diff() {
    local native="$1"
    local custom="$2"
    
    # Verificar que los valores no estén vacíos y sean números válidos
    if [ -z "$native" ] || [ -z "$custom" ]; then
        echo "N/A"
        return
    fi
    
    # Convertir a números y verificar que no sean cero
    if [ "$(echo "$native == 0" | bc -l 2>/dev/null)" = "1" ]; then
        if [ "$(echo "$custom == 0" | bc -l 2>/dev/null)" = "1" ]; then
            echo "0"
        else
            echo "N/A"
        fi
        return
    fi
    
    local diff=$(echo "scale=2; (($custom - $native) / $native) * 100" | bc -l 2>/dev/null)
    if [ $? -eq 0 ] && [ -n "$diff" ]; then
        echo "$diff"
    else
        echo "N/A"
    fi
}

# Función para mostrar comparación con colores
show_comparison() {
    local metric="$1"
    local native="$2"
    local custom="$3"
    local unit="$4"
    local diff="$5"
    
    printf "%-30s %15s %15s " "$metric" "$native$unit" "$custom$unit"
    
    if [ "$diff" == "PAGE" ] && [ -n "$diff" ]; then
		EXTRA_PAGES=$(echo "$3 - $2" | bc -l 2>/dev/null)
        if [ -n "$EXTRA_PAGES" ]; then
            local is_negative=$(echo "$EXTRA_PAGES < 0" | bc -l 2>/dev/null)
            local is_zero=$(echo "$EXTRA_PAGES == 0" | bc -l 2>/dev/null)
            
            if [ "$is_zero" = "1" ]; then
                printf "${YELLOW}%+.0f${NC}\n" "$EXTRA_PAGES"
            elif [ "$is_negative" = "1" ]; then
                printf "${GREEN}%+.0f${NC}\n" "$EXTRA_PAGES"
            else
                printf "${RED}%+.0f${NC}\n" "$EXTRA_PAGES"
            fi
        else
            printf "N/A\n"
        fi
    elif [ "$diff" != "N/A" ] && [ -n "$diff" ]; then
        # Usar bc para comparaciones numéricas seguras
        local is_negative=$(echo "$diff < 0" | bc -l 2>/dev/null)
        local is_high=$(echo "$diff > 10" | bc -l 2>/dev/null)
        
        if [ "$is_negative" = "1" ]; then
            printf "${GREEN}%+.1f%%${NC}\n" "$diff"
        elif [ "$is_high" = "1" ]; then
            printf "${RED}%+.1f%%${NC}\n" "$diff"
        else
            printf "${YELLOW}%+.1f%%${NC}\n" "$diff"
        fi
    else
        printf "%s\n" "$diff"
    fi
}

echo
printf "%-30s %15s %15s %10s\n" "METRIC" "NATIVE" "CUSTOM" "DIFFERENCE"
echo "$(printf '=%.0s' {1..75})"

# Extraer métricas
NATIVE_USER=$(extract_metric "$NATIVE_OUTPUT" "User time")
CUSTOM_USER=$(extract_metric "$CUSTOM_OUTPUT" "User time")
DIFF_USER=$(calculate_diff "$NATIVE_USER" "$CUSTOM_USER")

NATIVE_SYSTEM=$(extract_metric "$NATIVE_OUTPUT" "System time")
CUSTOM_SYSTEM=$(extract_metric "$CUSTOM_OUTPUT" "System time")
DIFF_SYSTEM=$(calculate_diff "$NATIVE_SYSTEM" "$CUSTOM_SYSTEM")

NATIVE_ELAPSED=$(time_to_seconds "$(extract_time "$NATIVE_OUTPUT")")
CUSTOM_ELAPSED=$(time_to_seconds "$(extract_time "$CUSTOM_OUTPUT")")
DIFF_ELAPSED=$(calculate_diff "$NATIVE_ELAPSED" "$CUSTOM_ELAPSED")

NATIVE_MEMORY=$(extract_metric "$NATIVE_OUTPUT" "Maximum resident set size")
CUSTOM_MEMORY=$(extract_metric "$CUSTOM_OUTPUT" "Maximum resident set size")
DIFF_MEMORY=$(calculate_diff "$NATIVE_MEMORY" "$CUSTOM_MEMORY")

NATIVE_PAGES=$(echo "scale=0; $NATIVE_MEMORY / 4" | bc -l 2>/dev/null)
CUSTOM_PAGES=$(echo "scale=0; $CUSTOM_MEMORY / 4" | bc -l 2>/dev/null)

NATIVE_MINOR=$(extract_metric "$NATIVE_OUTPUT" "Minor.*page faults")
CUSTOM_MINOR=$(extract_metric "$CUSTOM_OUTPUT" "Minor.*page faults")

NATIVE_MAJOR=$(extract_metric "$NATIVE_OUTPUT" "Major.*page faults")
CUSTOM_MAJOR=$(extract_metric "$CUSTOM_OUTPUT" "Major.*page faults")

NATIVE_VOLUNTARY=$(extract_metric "$NATIVE_OUTPUT" "Voluntary context switches")
CUSTOM_VOLUNTARY=$(extract_metric "$CUSTOM_OUTPUT" "Voluntary context switches")
DIFF_VOLUNTARY=$(calculate_diff "$NATIVE_VOLUNTARY" "$CUSTOM_VOLUNTARY")

# Show comparisons
show_comparison "User time" "$NATIVE_USER" "$CUSTOM_USER" "s" "$DIFF_USER"
show_comparison "System time" "$NATIVE_SYSTEM" "$CUSTOM_SYSTEM" "s" "$DIFF_SYSTEM"
show_comparison "Total time" "$NATIVE_ELAPSED" "$CUSTOM_ELAPSED" "s" "$DIFF_ELAPSED"
show_comparison "Maximum memory" "$NATIVE_MEMORY" "$CUSTOM_MEMORY" "KB" "$DIFF_MEMORY"
show_comparison "Total pages" "$NATIVE_PAGES" "$CUSTOM_PAGES" "" "PAGE"
show_comparison "Minor page faults" "$NATIVE_MINOR" "$CUSTOM_MINOR" "" "PAGE"
show_comparison "Major page faults" "$NATIVE_MAJOR" "$CUSTOM_MAJOR" "" "PAGE"
show_comparison "Voluntary context switches" "$NATIVE_VOLUNTARY" "$CUSTOM_VOLUNTARY" "" "$DIFF_VOLUNTARY"

echo

# Memory efficiency scoring (based on overhead percentage, not absolute pages)
if [ "$CUSTOM_PAGES" != "N/A" ] && [ "$NATIVE_PAGES" != "N/A" ] && [ -n "$CUSTOM_PAGES" ] && [ -n "$NATIVE_PAGES" ]; then
    overhead_pct=$(echo "scale=1; (($CUSTOM_PAGES - $NATIVE_PAGES) / $NATIVE_PAGES) * 100" | bc -l 2>/dev/null)
    
    if [ -n "$overhead_pct" ]; then
        is_negative=$(echo "$overhead_pct < 0" | bc -l 2>/dev/null)
        
        if [ "$is_negative" = "1" ]; then
            echo -e "${GREEN}Memory Score: 5/5 - Better than native malloc${NC}"
        elif [ "$(echo "$overhead_pct <= 10" | bc -l)" = "1" ]; then
            echo -e "${GREEN}Memory Score: 5/5 - Excellent overhead (≤10%)${NC}"
        elif [ "$(echo "$overhead_pct <= 25" | bc -l)" = "1" ]; then
            echo -e "${YELLOW}Memory Score: 4/5 - Good overhead (≤25%)${NC}"
        elif [ "$(echo "$overhead_pct <= 50" | bc -l)" = "1" ]; then
            echo -e "${YELLOW}Memory Score: 3/5 - Acceptable overhead (≤50%)${NC}"
        elif [ "$(echo "$overhead_pct <= 100" | bc -l)" = "1" ]; then
            echo -e "${RED}Memory Score: 2/5 - High overhead (≤100%)${NC}"
        else
            echo -e "${RED}Memory Score: 1/5 - Excessive overhead (>100%)${NC}"
        fi
    fi
fi

echo

# Basic automatic analysis
if [ "$DIFF_ELAPSED" != "N/A" ] && [ -n "$DIFF_ELAPSED" ]; then
    is_fast=$(echo "$DIFF_ELAPSED < -5" | bc -l 2>/dev/null)
    is_slow=$(echo "$DIFF_ELAPSED > 10" | bc -l 2>/dev/null)
    
    if [ "$is_fast" = "1" ]; then
        echo -e "${GREEN}✓ Your malloc is significantly faster${NC}"
    elif [ "$is_slow" = "1" ]; then
        echo -e "${RED}✗ Your malloc is notably slower${NC}"
    else
        echo -e "${YELLOW}≈ Similar performance in execution time${NC}"
    fi
fi

if [ "$DIFF_MEMORY" != "N/A" ] && [ -n "$DIFF_MEMORY" ]; then
    is_efficient=$(echo "$DIFF_MEMORY < -10" | bc -l 2>/dev/null)
    is_wasteful=$(echo "$DIFF_MEMORY > 20" | bc -l 2>/dev/null)
    
    if [ "$is_efficient" = "1" ]; then
        echo -e "${GREEN}✓ Your malloc uses less memory${NC}"
    elif [ "$is_wasteful" = "1" ]; then
        echo -e "${RED}✗ Your malloc uses considerably more memory${NC}"
    else
        echo -e "${YELLOW}≈ Similar memory usage${NC}"
    fi
fi

echo

# ssh koba@127.0.0.1 -p 2222
# scp -P 2222 -r D:\malloc koba@127.0.0.1:~/
# koba.a26*