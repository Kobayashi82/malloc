/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_main.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 18:14:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 20:51:17 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>

// Test colors
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define CYAN    "\033[0;36m"
#define NC      "\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

void test_assert(int condition, const char *test_name) {
    if (condition) {
        printf(GREEN "✓ " NC "%s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ " NC "%s\n", test_name);
        tests_failed++;
    }
}

void test_malloc_basic() {
    printf(CYAN "\n=== Testing malloc() - Basic functionality ===" NC "\n");
    
    // Test 1: Normal allocation
    void *ptr1 = malloc(100);
    test_assert(ptr1 != NULL, "malloc(100) returns non-NULL");
    
    // Test 2: Zero allocation (implementation defined)
    void *ptr2 = malloc(0);
    test_assert(ptr2 != NULL, "malloc(0) handled");
    
    // Test 3: Large allocation
    void *ptr3 = malloc(1024 * 1024);
    test_assert(ptr3 != NULL, "malloc(1MB) successful");
    
    // Test 4: Write to allocated memory
    if (ptr1) {
        memset(ptr1, 0x42, 100);
        test_assert(((char*)ptr1)[50] == 0x42, "Memory is writable");
    }
    
    // Test 5: Multiple allocations
    void *ptrs[10];
    int all_valid = 1;
    for (int i = 0; i < 10; i++) {
        ptrs[i] = malloc(100 + i * 10);
        if (!ptrs[i]) all_valid = 0;
    }
    test_assert(all_valid, "Multiple consecutive allocations");
    
    // Clean up
    free(ptr1);
    if (ptr2) free(ptr2);
    free(ptr3);
    for (int i = 0; i < 10; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }
    
    // Test 6: PERTURB functionality
    void *ptr_perturb = malloc(100);
    if (ptr_perturb) {
        // Check if memory is initialized with PERTURB value (42)
        int perturb_ok = 1;
        unsigned char *bytes = (unsigned char*)ptr_perturb;
        for (int i = 0; i < 100; i++) {
            if (bytes[i] != 42) {
                perturb_ok = 0;
                break;
            }
        }
        test_assert(perturb_ok, "malloc() PERTURB initialization (value 42)");
        free(ptr_perturb);
    }
}

void test_malloc_edge_cases() {
    printf(CYAN "\n=== Testing malloc() - Edge cases ===" NC "\n");
    
    // Test 1: Very large allocation (should fail gracefully)
    void *ptr1 = malloc(SIZE_MAX);
	if (ptr1 != NULL) write(2, "2\n", 2);
    test_assert(ptr1 == NULL, "malloc(SIZE_MAX) returns NULL");
    
    // Test 2: Near SIZE_MAX allocation
    void *ptr2 = malloc(SIZE_MAX - 1);
    test_assert(ptr2 == NULL, "malloc(SIZE_MAX-1) returns NULL");
    
    // Test 3: Power of 2 sizes
    size_t sizes[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    int all_pow2_valid = 1;
    void *pow2_ptrs[14];
    
    for (int i = 0; i < 14; i++) {
        pow2_ptrs[i] = malloc(sizes[i]);
        if (!pow2_ptrs[i]) all_pow2_valid = 0;
    }
    test_assert(all_pow2_valid, "Power of 2 allocations");
    
    // Test 4: Odd sizes
    size_t odd_sizes[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
    int all_odd_valid = 1;
    void *odd_ptrs[10];
    
    for (int i = 0; i < 10; i++) {
        odd_ptrs[i] = malloc(odd_sizes[i]);
        if (!odd_ptrs[i]) all_odd_valid = 0;
    }
    test_assert(all_odd_valid, "Odd size allocations");
    
    // Clean up
    for (int i = 0; i < 14; i++) {
        if (pow2_ptrs[i]) free(pow2_ptrs[i]);
    }
    for (int i = 0; i < 10; i++) {
        if (odd_ptrs[i]) free(odd_ptrs[i]);
    }
}

void test_free_basic() {
    printf(CYAN "\n=== Testing free() - Basic functionality ===" NC "\n");
    
    // Test 1: Free NULL (should not crash)
    free(NULL);
    test_assert(1, "free(NULL) doesn't crash");
    
    // Test 2: Free valid pointer
    void *ptr = malloc(100);
    if (ptr) {
        free(ptr);
        test_assert(1, "free(valid_ptr) doesn't crash");
    }
    
    // Test 3: Free multiple pointers
    void *ptrs[5];
    for (int i = 0; i < 5; i++) {
        ptrs[i] = malloc(100 + i * 10);
    }
    for (int i = 0; i < 5; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }
    test_assert(1, "Multiple free() calls don't crash");
}

void test_calloc_basic() {
    printf(CYAN "\n=== Testing calloc() - Basic functionality ===" NC "\n");
    
    // Test 1: Normal calloc
    void *ptr1 = calloc(10, 20);
    test_assert(ptr1 != NULL, "calloc(10, 20) returns non-NULL");
    
    // Test 2: Memory is zeroed
    if (ptr1) {
        int all_zero = 1;
        unsigned char *bytes = (unsigned char*)ptr1;
        for (int i = 0; i < 200; i++) {
            if (bytes[i] != 0) {
                all_zero = 0;
                break;
            }
        }
        test_assert(all_zero, "calloc() memory is zeroed");
    }
    
    // Test 3: Zero elements
    void *ptr2 = calloc(0, 20);
    test_assert(ptr2 == NULL || ptr2 != NULL, "calloc(0, 20) handled");
    
    // Test 4: Zero size
    void *ptr3 = calloc(20, 0);
    test_assert(ptr3 == NULL || ptr3 != NULL, "calloc(20, 0) handled");
    
    // Test 5: Overflow detection
    void *ptr4 = calloc(SIZE_MAX, 2);
    test_assert(ptr4 == NULL, "calloc() detects overflow");
    
    void *ptr5 = calloc(SIZE_MAX/2 + 1, 2);
    test_assert(ptr5 == NULL, "calloc() detects near-overflow");
    
    // Clean up
    if (ptr1) free(ptr1);
    if (ptr2) free(ptr2);
    if (ptr3) free(ptr3);
    
    // Test 6: PERTURB should NOT affect calloc (memory must remain zeroed)
    void *ptr_perturb = calloc(100, 1);
    if (ptr_perturb) {
        // Memory should still be zeroed despite PERTURB being enabled
        int all_zero = 1;
        unsigned char *bytes = (unsigned char*)ptr_perturb;
        for (int i = 0; i < 100; i++) {
            if (bytes[i] != 0) {
                all_zero = 0;
                break;
            }
        }
        test_assert(all_zero, "calloc() ignores PERTURB and keeps memory zeroed");
        free(ptr_perturb);
    }
}

void test_realloc_basic() {
    printf(CYAN "\n=== Testing realloc() - Basic functionality ===" NC "\n");
    
    // Test 1: realloc with NULL (should behave like malloc)
    void *ptr1 = realloc(NULL, 100);
    test_assert(ptr1 != NULL, "realloc(NULL, 100) works like malloc");
    
    // Test 2: realloc with size 0 (should behave like free)
    void *ptr2 = malloc(100);
    void *ptr3 = realloc(ptr2, 0);
    test_assert(ptr3 == NULL, "realloc(ptr, 0) behaves like free");
    
    // Test 3: Expand allocation
    void *ptr4 = malloc(100);
    if (ptr4) {
        memset(ptr4, 0x42, 100);
        void *ptr5 = realloc(ptr4, 200);
        if (ptr5) {
            // Check that original data is preserved
            int data_preserved = 1;
            unsigned char *bytes = (unsigned char*)ptr5;
            for (int i = 0; i < 100; i++) {
                if (bytes[i] != 0x42) {
                    data_preserved = 0;
                    break;
                }
            }
            test_assert(data_preserved, "realloc() preserves data when expanding");
            free(ptr5);
        } else {
            free(ptr4);
            test_assert(0, "realloc() expansion failed");
        }
    }
    
    // Test 4: Shrink allocation
    void *ptr6 = malloc(1000);
    if (ptr6) {
        memset(ptr6, 0x33, 1000);
        void *ptr7 = realloc(ptr6, 100);
        if (ptr7) {
            int data_preserved = 1;
            unsigned char *bytes = (unsigned char*)ptr7;
            for (int i = 0; i < 100; i++) {
                if (bytes[i] != 0x33) {
                    data_preserved = 0;
                    break;
                }
            }
            test_assert(data_preserved, "realloc() preserves data when shrinking");
            free(ptr7);
        } else {
            test_assert(0, "realloc() shrinking failed");
        }
    }
    
    // Clean up
    if (ptr1) free(ptr1);
    
    // Test 5: PERTURB functionality
    void *ptr_perturb = realloc(NULL, 100);  // Acts like malloc
    if (ptr_perturb) {
        // Check if memory is initialized with PERTURB value (42)
        int perturb_ok = 1;
        unsigned char *bytes = (unsigned char*)ptr_perturb;
        for (int i = 0; i < 100; i++) {
            if (bytes[i] != 42) {
                perturb_ok = 0;
                break;
            }
        }
        test_assert(perturb_ok, "realloc() PERTURB initialization (value 42)");
        
        // Test expansion with PERTURB
        void *ptr_expanded = realloc(ptr_perturb, 200);
        if (ptr_expanded) {
            // New memory (from 100 to 200) should be initialized with PERTURB value
            unsigned char *new_bytes = (unsigned char*)ptr_expanded;
            int new_perturb_ok = 1;
            for (int i = 100; i < 200; i++) {
                if (new_bytes[i] != 42) {
                    new_perturb_ok = 0;
                    break;
                }
            }
            test_assert(new_perturb_ok, "realloc() expansion PERTURB initialization");
            free(ptr_expanded);
        } else {
            free(ptr_perturb);
        }
    }
}

void test_integration() {
    printf(CYAN "\n=== Integration tests ===" NC "\n");
    
    // Test 1: Mixed operations
    void *ptr1 = malloc(100);
    void *ptr2 = calloc(50, 2);
    void *ptr3 = realloc(ptr1, 200);
    
    test_assert(ptr2 != NULL && ptr3 != NULL, "Mixed malloc/calloc/realloc");
    
    free(ptr2);
    free(ptr3);
    
    // Test 2: Stress test - many allocations
    void *ptrs[1000];
    int stress_success = 1;
    
    for (int i = 0; i < 1000; i++) {
        ptrs[i] = malloc(i + 1);
        if (!ptrs[i] && i + 1 > 0) {
            stress_success = 0;
            break;
        }
    }
    
    for (int i = 0; i < 1000; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }
    
    test_assert(stress_success, "Stress test: 1000 allocations");
    
    // Test 3: Fragmentation test
    void *frag_ptrs[100];
    
    // Allocate
    for (int i = 0; i < 100; i++) {
        frag_ptrs[i] = malloc(100);
    }
    
    // Free every other one
    for (int i = 1; i < 100; i += 2) {
        free(frag_ptrs[i]);
        frag_ptrs[i] = NULL;
    }
    
    // Try to allocate again
    for (int i = 1; i < 100; i += 2) {
        frag_ptrs[i] = malloc(50);
    }
    
    int frag_success = 1;
    for (int i = 0; i < 100; i++) {
        if (!frag_ptrs[i]) frag_success = 0;
    }
    
    test_assert(frag_success, "Fragmentation handling");
    
    // Clean up
    for (int i = 0; i < 100; i++) {
        if (frag_ptrs[i]) free(frag_ptrs[i]);
    }
}

int main() {   
    test_malloc_basic();
    test_malloc_edge_cases();
    test_free_basic();
    test_calloc_basic();
    test_realloc_basic();
    test_integration();
}
