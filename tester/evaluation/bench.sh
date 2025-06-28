#!/bin/bash

echo

if [ $# -eq 0 ]; then
    echo "Usage: $0 <program> [arguments...]"
	echo
    exit 1
fi

PROGRAM="$1"
shift
ARGS="$@"

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

if ! command -v "$PROGRAM" >/dev/null 2>&1 && [ ! -f "$PROGRAM" ]; then
    echo -e "${RED}Error: Cannot find program '$PROGRAM'${NC}"
	echo
    exit 1
fi

NATIVE_OUTPUT=$(mktemp)
CUSTOM_OUTPUT=$(mktemp)

echo -e "Program: ${YELLOW}$PROGRAM $ARGS${NC}"

cleanup() {
    rm -f "$NATIVE_OUTPUT" "$CUSTOM_OUTPUT"
}
trap cleanup EXIT

/usr/bin/time -v "$PROGRAM" $ARGS 2>"$NATIVE_OUTPUT" >/dev/null

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Failed execution with native malloc${NC}"
    exit 1
fi

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

extract_time() {
    local file="$1"
    local time_line=$(grep "Elapsed (wall clock) time" "$file" | awk -F': ' '{print $2}')
    if [ -z "$time_line" ]; then
        echo "0"
    else
        echo "$time_line"
    fi
}

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

calculate_diff() {
    local native="$1"
    local custom="$2"
    
    if [ -z "$native" ] || [ -z "$custom" ]; then
        echo "N/A"
        return
    fi
    
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
                printf "\n"
            elif [ "$is_negative" = "1" ]; then
                printf "${GREEN}%+.0f${NC}\n" "$EXTRA_PAGES"
            else
                printf "${RED}%+.0f${NC}\n" "$EXTRA_PAGES"
            fi
        else
            printf "N/A\n"
        fi
    elif [ "$diff" == "KB" ] && [ -n "$diff" ]; then
        MEMORY_DIFF=$(echo "$3 - $2" | bc -l 2>/dev/null)
        if [ -n "$MEMORY_DIFF" ]; then
            local is_negative=$(echo "$MEMORY_DIFF < 0" | bc -l 2>/dev/null)
            local is_zero=$(echo "$MEMORY_DIFF == 0" | bc -l 2>/dev/null)
            
            if [ "$is_zero" = "1" ]; then
                printf "\n"
            elif [ "$is_negative" = "1" ]; then
                printf "${GREEN}%+.0fKB${NC}\n" "$MEMORY_DIFF"
            else
                printf "${RED}%+.0fKB${NC}\n" "$MEMORY_DIFF"
            fi
        else
            printf "N/A\n"
        fi
    elif [ "$diff" != "N/A" ] && [ -n "$diff" ]; then
        local is_negative=$(echo "$diff < 0" | bc -l 2>/dev/null)
        local is_zero=$(echo "$diff == 0" | bc -l 2>/dev/null)
        local is_high=$(echo "$diff > 10" | bc -l 2>/dev/null)
        
        if [ "$is_zero" = "1" ]; then
            printf "\n"
        elif [ "$is_negative" = "1" ]; then
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

show_comparison "User time" "$NATIVE_USER" "$CUSTOM_USER" "s" "$DIFF_USER"
show_comparison "System time" "$NATIVE_SYSTEM" "$CUSTOM_SYSTEM" "s" "$DIFF_SYSTEM"
show_comparison "Total time" "$NATIVE_ELAPSED" "$CUSTOM_ELAPSED" "s" "$DIFF_ELAPSED"
show_comparison "Maximum memory" "$NATIVE_MEMORY" "$CUSTOM_MEMORY" "KB" "KB"
show_comparison "Total pages" "$NATIVE_PAGES" "$CUSTOM_PAGES" "" "PAGE"
show_comparison "Minor page faults" "$NATIVE_MINOR" "$CUSTOM_MINOR" "" "PAGE"
show_comparison "Major page faults" "$NATIVE_MAJOR" "$CUSTOM_MAJOR" "" "PAGE"
show_comparison "Voluntary context switches" "$NATIVE_VOLUNTARY" "$CUSTOM_VOLUNTARY" "" "$DIFF_VOLUNTARY"

echo

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
