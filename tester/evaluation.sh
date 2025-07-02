#!/bin/bash

# ────────────── Get script directory ──────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ────────────── Colors ───────────────────
RED='\033[0;31m'; BROWN='\033[0;33m'; YELLOW='\033[1;33m'; GREEN='\033[0;32m'; BLUE='\033[0;34m'; CYAN='\033[0;36m'; MAGENTA='\033[1;35m'; GREENB='\033[1;32m'; NC='\033[0m'

# ────────────── Compilation ──────────────
tests=(test0 test1 test2 test3 test4 test5)
for t in "${tests[@]}"; do
	file="${SCRIPT_DIR}/evaluation/${t}.c"
	[[ -f "$file" ]] || { echo -e "${RED}Missing $file${NC}"; exit 1; }
	
	if [[ "$t" == "test5" ]]; then
		clang "$file" -o "${SCRIPT_DIR}/evaluation/$t" -L"${SCRIPT_DIR}/../build/lib" -I"${SCRIPT_DIR}/../inc" -lft_malloc || { echo -e "${RED}Compilation failed $file${NC}"; exit 1; }
	else
		clang "$file" -o "${SCRIPT_DIR}/evaluation/$t" || { echo -e "${RED}Compilation failed $file${NC}"; exit 1; }
	fi
done
echo

# ────────────── Helpers ──────────────────
run() {
	tmp=$(mktemp)
	stdout_tmp=$(mktemp)
	/usr/bin/time -v "$1" 2>"$tmp" >"$stdout_tmp"
	exit_code=$?
	mem=$(grep "Maximum resident" "$tmp" | awk '{print $NF}')
	minor=$(grep "Minor" "$tmp" | awk '{print $NF}')
	
	# Check if stdout has two identical lines (for test3)
	realloc_ok=0
	if [[ "$1" == *"test3"* ]]; then
		lines=($(cat "$stdout_tmp"))
		if [[ ${#lines[@]} -eq 2 && "${lines[0]}" == "${lines[1]}" ]]; then
			realloc_ok=1
		fi
	fi
	
	# Check if process aborted (for test4)
	abort_ok=0
	if [[ "$1" == *"test4"* ]]; then
		if (( exit_code != 0 )); then
			abort_ok=1
		fi
	fi
	
	rm -f "$tmp" "$stdout_tmp"
	((mem==0)) && echo "0 0 0 0 0" && return
	echo "$mem $((mem/4)) $minor $realloc_ok $abort_ok"
}

rate_pages() {
	p=$1
	((p<255)) && echo 0 && return
	((p>=1024)) && echo 1 && return
	((p>=512)) && echo 2 && return
	((p>=342)) && echo 3 && return
	((p>=291)) && echo 4 && return
	echo 5
}

rate_free() {
	local t2_min=$1 t0_min=$2 t1_min=$3
	local diff=$(( t2_min - t0_min ))
	local t1_diff=$(( t1_min - t0_min ))
	
	(( t2_min >= t1_min )) && echo 0 && return
	
	(( diff <= 3 )) && echo 5 && return
	(( diff <= 6 )) && echo 4 && return
	(( diff <= 9 )) && echo 3 && return
	(( diff <= 12 )) && echo 2 && return
	echo 1
}

get_color() {
	local score=$1 max_score=$2
	local percent=$(( score * 100 / max_score ))
	
	(( percent >= 90 )) && echo "$GREEN" && return
	(( percent >= 70 )) && echo "$YELLOW" && return
	(( percent >= 30 )) && echo "$BROWN" && return
	echo "$RED"
}

# ────────────── Executions ───────────────
declare -A N_MEM N_PAGES N_MIN N_REALLOC N_ABORT C_MEM C_PAGES C_MIN C_REALLOC C_ABORT
for t in "${tests[@]}"; do
	# native malloc
	read mem pages minor realloc abort < <(run "${SCRIPT_DIR}/evaluation/$t")
	N_MEM[$t]=$mem; N_PAGES[$t]=$pages; N_MIN[$t]=$minor; N_REALLOC[$t]=$realloc; N_ABORT[$t]=$abort
	# custom malloc
	unset MALLOC_DEBUG MALLOC_LOGGING
	export LD_LIBRARY_PATH="${SCRIPT_DIR}/../build/lib:$LD_LIBRARY_PATH"
	export LD_PRELOAD="libft_malloc.so"
	export MALLOC_CHECK_=2;
	read mem pages minor realloc abort < <(run "${SCRIPT_DIR}/evaluation/$t")
	C_MEM[$t]=$mem; C_PAGES[$t]=$pages; C_MIN[$t]=$minor; C_REALLOC[$t]=$realloc; C_ABORT[$t]=$abort
	unset LD_PRELOAD
	unset MALLOC_CHECK_;
done

BASE_PAGES=${C_PAGES[test0]}
BASE_MIN=${C_MIN[test0]}

# ==================================================================================================
#                                          NATIVE MALLOC
# ==================================================================================================

echo -e "\t\t\t\t   ${MAGENTA}NATIVE MALLOC${NC}"
echo
echo -en "${CYAN}"
printf "%-5s %12s %10s %10s | %12s %10s %10s\n" "TEST" "Memory (KB)" "Pages" "Minor" "Memory (KB)" "Pages" "Minor"
echo -e "${CYAN}------------------------------------------------------------------------------${NC}"
printf "%-5s ${GREEN}%12d${NC} ${YELLOW}%10d${NC} ${BROWN}%10d${NC} |${GREEN}%13s${NC} ${YELLOW}%10s${NC} ${BROWN}%10s${NC}\n" "test0" "${N_MEM[test0]}" "${N_PAGES[test0]}" "${N_MIN[test0]}" "-" "-" "-"

for t in test1 test2 test3 test4; do
	delta_mem=$(( N_MEM[$t] - N_MEM[test0] ))
	delta_pages=$(( N_PAGES[$t] - N_PAGES[test0] ))
	delta_min=$(( N_MIN[$t] - N_MIN[test0] ))
	
	printf "%-5s ${GREEN}%12d${NC} ${YELLOW}%10d${NC} ${BROWN}%10d${NC} | ${GREEN}%+12d${NC} ${YELLOW}%+10d${NC} ${BROWN}%+10d${NC}\n" "$t" "${N_MEM[$t]}" "${N_PAGES[$t]}" "${N_MIN[$t]}" "$delta_mem" "$delta_pages" "$delta_min"
done
echo

# ────────────── Score ────────────────────
delta_pages_test1_native=$(( N_PAGES[test1] - N_PAGES[test0] ))
score1_native=$(rate_pages "$delta_pages_test1_native")
color1_native=$(get_color $score1_native 5)
echo -e "Memory score: ${color1_native}${score1_native}/5${NC} (${delta_pages_test1_native} pages)"

free_score_native=$(rate_free "${N_MIN[test2]}" "${N_MIN[test0]}" "${N_MIN[test1]}")
color_free_native=$(get_color $free_score_native 5)
delta_test2_native=$(( N_MIN[test2] - N_MIN[test0] ))
if (( delta_test2_native > 0 )); then
	delta_str_native="+${delta_test2_native}"
elif (( delta_test2_native == 0 )); then
	delta_str_native="+0"
else
	delta_str_native="${delta_test2_native}"
fi
echo -e "Free quality: ${color_free_native}${free_score_native}/5${NC} (${delta_str_native} pages)"

# Realloc status
if (( N_REALLOC[test3] == 1 )); then
	echo -e "Realloc:      ${GREEN}✓${NC}"
else
	echo -e "Realloc:      ${RED}X${NC}"
fi

# Abort test status
if (( N_ABORT[test4] == 1 )); then
	echo -e "Error Handle: ${RED}X${NC}"
else
	echo -e "Error Handle: ${GREEN}✓${NC}"
fi

echo

# ==================================================================================================
#                                          CUSTOM MALLOC
# ==================================================================================================

echo -e "\t\t\t\t   ${GREENB}CUSTOM MALLOC${NC}"
echo
echo -en "${CYAN}"
printf "%-5s %12s %10s %10s | %12s %10s %10s\n" "TEST" "Memory (KB)" "Pages" "Minor" "Memory (KB)" "Pages" "Minor"
echo -e "${CYAN}------------------------------------------------------------------------------${NC}"
printf "%-5s ${GREEN}%12d${NC} ${YELLOW}%10d${NC} ${BROWN}%10d${NC} |${GREEN}%13s${NC} ${YELLOW}%10s${NC} ${BROWN}%10s${NC}\n" "test0" "${C_MEM[test0]}" "${C_PAGES[test0]}" "${C_MIN[test0]}" "-" "-" "-"

for t in test1 test2 test3 test4; do
	delta_mem=$(( C_MEM[$t] - C_MEM[test0] ))
	delta_pages=$(( C_PAGES[$t] - C_PAGES[test0] ))
	delta_min=$(( C_MIN[$t] - C_MIN[test0] ))
	
	printf "%-5s ${GREEN}%12d${NC} ${YELLOW}%10d${NC} ${BROWN}%10d${NC} | ${GREEN}%+12d${NC} ${YELLOW}%+10d${NC} ${BROWN}%+10d${NC}\n" "$t" "${C_MEM[$t]}" "${C_PAGES[$t]}" "${C_MIN[$t]}" "$delta_mem" "$delta_pages" "$delta_min"
done
echo

# ────────────── Score ────────────────────
delta_pages_test1=$(( C_PAGES[test1] - C_PAGES[test0] ))
score1=$(rate_pages "$delta_pages_test1")
color1=$(get_color $score1 5)
echo -e "Memory score: ${color1}${score1}/5${NC} (${delta_pages_test1} pages)"

free_score=$(rate_free "${C_MIN[test2]}" "${C_MIN[test0]}" "${C_MIN[test1]}")
color_free=$(get_color $free_score 5)
delta_test2=$(( C_MIN[test2] - C_MIN[test0] ))
if (( delta_test2 > 0 )); then
	delta_str="+${delta_test2}"
elif (( delta_test2 == 0 )); then
	delta_str="+0"
else
	delta_str="${delta_test2}"
fi
echo -e "Free quality: ${color_free}${free_score}/5${NC} (${delta_str} pages)"

# Realloc status
if (( C_REALLOC[test3] == 1 )); then
	echo -e "Realloc:      ${GREEN}✓${NC}"
else
	echo -e "Realloc:      ${RED}X${NC}"
fi

# Abort test status
if (( C_ABORT[test4] == 1 )); then
	echo -e "Error Handle: ${RED}X${NC}"
else
	echo -e "Error Handle: ${GREEN}✓${NC}"
fi

# Show Alloc Mem
echo
echo -e "  ${CYAN}SHOW_ALLOC_MEM${YELLOW}${NC}"
echo -e "${CYAN}------------------${NC}"
echo

# "${SCRIPT_DIR}/evaluation/test5"
echo

rm -f "${SCRIPT_DIR}/evaluation/test0" "${SCRIPT_DIR}/evaluation/test1" "${SCRIPT_DIR}/evaluation/test2" "${SCRIPT_DIR}/evaluation/test3" "${SCRIPT_DIR}/evaluation/test4" "${SCRIPT_DIR}/evaluation/test5"