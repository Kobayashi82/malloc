# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/02 18:12:47 by vzurera-          #+#    #+#              #
#    Updated: 2025/07/02 23:38:40 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Colors for output
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
CYAN = \033[0;36m
NC = \033[0m

# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -Werror -g -pthread
INCLUDES = -I../../inc

# Test executables
TESTS = test_main test_alignment test_extra test_stress run_all_tests

# Source files
MAIN_SRC = test_main.c
ALIGNMENT_SRC = test_alignment.c
EXTRA_SRC = test_extra.c
STRESS_SRC = test_stress.c
RUN_ALL_SRC = run_all_tests.c

# Default target
all: $(TESTS)
	@echo "$(GREEN)All tests compiled successfully!$(NC)"
	@echo "$(YELLOW)To run all tests: ./run_all_tests$(NC)"
	@echo "$(YELLOW)To run individual tests:$(NC)"
	@echo "  ./test_main      - Basic malloc functions"
	@echo "  ./test_alignment - Alignment functions"
	@echo "  ./test_extra     - Extra functions"
	@echo "  ./test_stress    - Stress and edge cases"

all_silent: $(TESTS)

# Individual test targets
test_main: $(MAIN_SRC)
	@echo "$(CYAN)Compiling test_main...$(NC)"
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

test_alignment: $(ALIGNMENT_SRC)
	@echo "$(CYAN)Compiling test_alignment...$(NC)"
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

test_extra: $(EXTRA_SRC)
	@echo "$(CYAN)Compiling test_extra...$(NC)"
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

test_stress: $(STRESS_SRC)
	@echo "$(CYAN)Compiling test_stress...$(NC)"
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -lpthread

run_all_tests: $(RUN_ALL_SRC)
	@echo "$(CYAN)Compiling run_all_tests...$(NC)"
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

# Quick test - compile and run
quick: all
	@echo "$(BLUE)Running quick test suite...$(NC)"
	./run_all_tests

# Clean targets
clean:
	@echo "$(YELLOW)Cleaning test executables...$(NC)"
	rm -f $(TESTS)

fclean: clean
	@echo "$(YELLOW)Full clean completed.$(NC)"

re: fclean all

# Run individual test suites
run-main: test_main
	@echo "$(BLUE)Running main functions test...$(NC)"
	./test_main

run-alignment: test_alignment
	@echo "$(BLUE)Running alignment functions test...$(NC)"
	./test_alignment

run-extra: test_extra
	@echo "$(BLUE)Running extra functions test...$(NC)"
	./test_extra

run-stress: test_stress
	@echo "$(BLUE)Running stress test...$(NC)"
	./test_stress

# Help target
help:
	@echo "$(BLUE)Available targets:$(NC)"
	@echo "  all           - Compile all tests"
	@echo "  quick         - Compile and run all tests"
	@echo "  test_main     - Compile main functions test"
	@echo "  test_alignment- Compile alignment functions test"
	@echo "  test_extra    - Compile extra functions test"
	@echo "  test_stress   - Compile stress test"
	@echo "  run_all_tests - Compile test runner"
	@echo "  run-main      - Compile and run main test"
	@echo "  run-alignment - Compile and run alignment test"
	@echo "  run-extra     - Compile and run extra test"
	@echo "  run-stress    - Compile and run stress test"
	@echo "  clean         - Remove executables"
	@echo "  fclean        - Full clean"
	@echo "  re            - Clean and rebuild"
	@echo "  help          - Show this help"

.PHONY: all all_silent clean fclean re quick run-main run-alignment run-extra run-stress help
