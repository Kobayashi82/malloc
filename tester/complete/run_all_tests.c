/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_all_tests.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 18:14:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 23:12:56 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

// Test colors
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define CYAN    "\033[0;36m"
#define MAGENTA "\033[0;35m"
#define NC      "\033[0m"

typedef struct {
    const char *name;
    const char *executable;
    const char *description;
} test_suite_t;

int run_test_suite(const char *name, const char *executable, const char *description) {
    printf(CYAN "\n================================================" NC "\n");
    printf(CYAN "Running: %s" NC "\n", name);
    printf(CYAN "Description: %s" NC "\n", description);
    printf(CYAN "================================================" NC "\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - run the test
        execl(executable, executable, NULL);
        printf(RED "Failed to execute %s" NC "\n", executable);
        exit(1);
    } else if (pid > 0) {
        // Parent process - wait for child
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == 0) {
                printf(GREEN "\n✓ %s" NC "\n", name);
                return 0;
            } else {
                printf(RED "\n✗ %s" NC "\n", name);
                return 1;
            }
        } else {
            printf(RED "\n✗ %s CRASHED" NC "\n", name);
            return 1;
        }
    } else {
        printf(RED "Failed to fork process for %s" NC "\n", name);
        return 1;
    }
}

void print_header() {
    printf(YELLOW "This test suite will thoroughly test your malloc implementation\n");
    printf("including all main functions, alignment functions, extra functions,\n");
    printf("and stress/edge case scenarios.\n" NC);
    printf("\n");
    printf(MAGENTA "Make sure your malloc library is loaded with: " YELLOW "source ../load.sh\n" NC);
}

void print_summary(int failed_tests) {
    printf(CYAN "\n========================================" NC "\n");
    printf("\n");
    
    if (failed_tests == 0) {
        printf(GREEN "\t🎉 ALL TESTS PASSED! 🎉" NC "\n");
    } else {
        printf(RED "\t❌ SOME TESTS FAILED ❌" NC "\n");
    }
    printf("\n");
}

int main() {
    print_header();
    
    test_suite_t test_suites[] = {
        {
            "Main Functions Test",
            "./test_main",
            "Tests malloc, free, calloc, realloc basic functionality"
        },
        {
            "Alignment Functions Test", 
            "./test_alignment",
            "Tests aligned_alloc, memalign, posix_memalign, valloc, pvalloc"
        },
        {
            "Extra Functions Test",
            "./test_extra", 
            "Tests reallocarray, malloc_usable_size and edge cases"
        },
        {
            "Stress & Edge Cases Test",
            "./test_stress",
            "Tests performance, threading, fragmentation, and error conditions"
        }
    };
    
    const int num_suites = sizeof(test_suites) / sizeof(test_suites[0]);
    int passed = 0;
    int failed = 0;
    
    // Check if all test executables exist
    for (int i = 0; i < num_suites; i++) {
        if (access(test_suites[i].executable, X_OK) != 0) {
            printf(RED "ERROR: Test executable " YELLOW "%s" RED " not found or not executable!" NC "\n", test_suites[i].executable);
            printf(CYAN "Please compile all tests first with:" YELLOW "make tests" NC "\n");
            return 1;
        }
    }
    
    // Run all test suites
    for (int i = 0; i < num_suites; i++) {
        if (run_test_suite(test_suites[i].name, 
                          test_suites[i].executable, 
                          test_suites[i].description) == 0) {
            passed++;
        } else {
            failed++;
        }
        
        // Small delay between tests
        // sleep(1);
    }
    
	(void) passed;
    print_summary(failed);
    
    return (failed == 0) ? 0 : 1;
}
