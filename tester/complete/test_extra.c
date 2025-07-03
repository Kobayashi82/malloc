/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_extra.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 18:14:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 20:09:00 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <malloc.h>

// Function declarations for our custom malloc functions
extern void *reallocarray(void *ptr, size_t nmemb, size_t size);
extern size_t malloc_usable_size(void *ptr);
extern void *memalign(size_t alignment, size_t size);
extern void *aligned_alloc(size_t alignment, size_t size);

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

void test_reallocarray() {
    printf(CYAN "\n=== Testing reallocarray() ===" NC "\n");
    
    // Test 1: Basic functionality (like realloc but with overflow protection)
    void *ptr1 = reallocarray(NULL, 10, 20);
    test_assert(ptr1 != NULL, "reallocarray(NULL, 10, 20) works like malloc");
    
    if (ptr1) {
        memset(ptr1, 0x11, 200);
        void *ptr2 = reallocarray(ptr1, 15, 20);
        if (ptr2) {
            // Check data preservation
            int data_preserved = 1;
            unsigned char *bytes = (unsigned char*)ptr2;
            for (int i = 0; i < 200; i++) {
                if (bytes[i] != 0x11) {
                    data_preserved = 0;
                    break;
                }
            }
            test_assert(data_preserved, "reallocarray() preserves data when expanding");
            free(ptr2);
        } else {
            free(ptr1);
            test_assert(0, "reallocarray() expansion failed");
        }
    }
    
    // Test 2: Overflow detection
    void *ptr3 = reallocarray(NULL, SIZE_MAX, 2);
    test_assert(ptr3 == NULL, "reallocarray() detects overflow (SIZE_MAX * 2)");
    
    void *ptr4 = reallocarray(NULL, SIZE_MAX/2 + 1, 2);
    test_assert(ptr4 == NULL, "reallocarray() detects near-overflow");
    
    // Test 3: Zero elements
    void *ptr5 = malloc(100);
    void *ptr6 = reallocarray(ptr5, 0, 20);
    test_assert(ptr6 == NULL, "reallocarray(ptr, 0, size) frees memory");
    
    // Test 4: Zero size
    void *ptr7 = malloc(100);
    void *ptr8 = reallocarray(ptr7, 20, 0);
    test_assert(ptr8 == NULL, "reallocarray(ptr, nmemb, 0) frees memory");
    
    // Test 5: Large but valid allocation
    void *ptr9 = reallocarray(NULL, 1000, 1000);
    test_assert(ptr9 != NULL, "reallocarray() large valid allocation");
    if (ptr9) free(ptr9);
    
    // Test 6: Shrinking
    void *ptr10 = reallocarray(NULL, 100, 50);  // 5000 bytes
    if (ptr10) {
        memset(ptr10, 0x22, 5000);
        void *ptr11 = reallocarray(ptr10, 50, 30);  // 1500 bytes
        if (ptr11) {
            int data_preserved = 1;
            unsigned char *bytes = (unsigned char*)ptr11;
            for (int i = 0; i < 1500; i++) {
                if (bytes[i] != 0x22) {
                    data_preserved = 0;
                    break;
                }
            }
            test_assert(data_preserved, "reallocarray() preserves data when shrinking");
            free(ptr11);
        } else {
            test_assert(0, "reallocarray() shrinking failed");
        }
    }
    
    // Test 7: Edge case - very small numbers that multiply to larger
    void *ptr12 = reallocarray(NULL, 2, 3);
    test_assert(ptr12 != NULL, "reallocarray() small numbers");
    if (ptr12) free(ptr12);
    
    // Test 8: PERTURB functionality
    void *ptr13 = malloc(10);
	ptr13 = reallocarray(NULL, 10, 10);  // 100 bytes
    if (ptr13) {
        // Check if memory is initialized with PERTURB value (42)
        int perturb_ok = 1;
        unsigned char *bytes = (unsigned char*)ptr13;
        for (int i = 0; i < 100; i++) {
            if (bytes[i] != 42) {
                perturb_ok = 0;
                break;
            }
        }
        test_assert(perturb_ok, "reallocarray() PERTURB initialization (value 42)");

        // Write some data and then free to test PERTURB on free
        memset(ptr13, 0xAA, 100);
        free(ptr13);

        // After free, memory should be set to 42^255 = 213
        // Note: This test might not work reliably since freed memory might be reused
        // but we test the concept
    }
}

void test_malloc_usable_size() {
    printf(CYAN "\n=== Testing malloc_usable_size() ===" NC "\n");
    
    // Test 1: Basic functionality
    void *ptr1 = malloc(100);
    if (ptr1) {
        size_t usable1 = malloc_usable_size(ptr1);
        test_assert(usable1 >= 100, "malloc_usable_size() >= requested size");
        free(ptr1);
    }
    
    // Test 2: NULL pointer
    size_t usable_null = malloc_usable_size(NULL);
    test_assert(usable_null == 0, "malloc_usable_size(NULL) returns 0");
    
    // Test 3: Different allocation sizes
    size_t sizes[] = {1, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};
    int size_test_passed = 1;
    
    for (int i = 0; i < 10; i++) {
        void *ptr = malloc(sizes[i]);
        if (ptr) {
            size_t usable = malloc_usable_size(ptr);
            if (usable < sizes[i]) {
                size_test_passed = 0;
            }
            free(ptr);
        } else {
            size_test_passed = 0;
        }
    }
    test_assert(size_test_passed, "malloc_usable_size() correct for various sizes");
    
    // Test 4: Calloc
    void *ptr2 = calloc(50, 4);  // 200 bytes
    if (ptr2) {
        size_t usable2 = malloc_usable_size(ptr2);
        test_assert(usable2 >= 200, "malloc_usable_size() works with calloc");
        free(ptr2);
    }
    
    // Test 5: Realloc
    void *ptr3 = malloc(100);
    if (ptr3) {
        void *ptr4 = realloc(ptr3, 300);
        if (ptr4) {
            size_t usable3 = malloc_usable_size(ptr4);
            test_assert(usable3 >= 300, "malloc_usable_size() works with realloc");
            free(ptr4);
        } else {
            free(ptr3);
        }
    }
    
    // Test 6: Aligned allocations
    void *ptr5 = memalign(64, 200);
    if (ptr5) {
        size_t usable4 = malloc_usable_size(ptr5);
        test_assert(usable4 >= 200, "malloc_usable_size() works with memalign");
        free(ptr5);
    }
    
    // Test 7: Write to full usable size
    void *ptr6 = malloc(100);
    if (ptr6) {
        size_t usable5 = malloc_usable_size(ptr6);
        // Try to write to the full usable size
        memset(ptr6, 0x33, usable5);
        
        // Verify we can read it back
        int memory_ok = 1;
        unsigned char *bytes = (unsigned char*)ptr6;
        for (size_t i = 0; i < usable5; i++) {
            if (bytes[i] != 0x33) {
                memory_ok = 0;
                break;
            }
        }
        test_assert(memory_ok, "Full usable size is writable");
        free(ptr6);
    }
}

void test_edge_cases() {
    printf(CYAN "\n=== Testing edge cases ===" NC "\n");
    
    // Test 1: Very large reallocarray that should fail
    void *ptr1 = reallocarray(NULL, SIZE_MAX/2, SIZE_MAX/2);
    test_assert(ptr1 == NULL, "reallocarray() rejects huge multiplication");
    
    // Test 2: One large, one small that overflows
    void *ptr2 = reallocarray(NULL, SIZE_MAX, 1);
    test_assert(ptr2 == NULL, "reallocarray(SIZE_MAX, 1) fails");
    
    void *ptr3 = reallocarray(NULL, 1, SIZE_MAX);
    test_assert(ptr3 == NULL, "reallocarray(1, SIZE_MAX) fails");
    
    // Test 3: Boundary cases for reallocarray
    void *ptr4 = reallocarray(NULL, SIZE_MAX/4, 3);  // Might overflow
    test_assert(ptr4 == NULL, "reallocarray() overflow boundary test");
    
    // Test 4: malloc_usable_size consistency
    void *ptr5 = malloc(1000);
    if (ptr5) {
        size_t usable1 = malloc_usable_size(ptr5);
        size_t usable2 = malloc_usable_size(ptr5);
        test_assert(usable1 == usable2, "malloc_usable_size() is consistent");
        free(ptr5);
    }
    
    // Test 5: malloc_usable_size after partial write
    void *ptr6 = malloc(500);
    if (ptr6) {
        memset(ptr6, 0x44, 250);  // Only write to half
        size_t usable = malloc_usable_size(ptr6);
        test_assert(usable >= 500, "malloc_usable_size() unaffected by partial write");
        free(ptr6);
    }
    
    // Test 6: reallocarray preserving data across size calculations
    void *ptr7 = reallocarray(NULL, 10, 10);  // 100 bytes
    if (ptr7) {
        memset(ptr7, 0x55, 100);
        void *ptr8 = reallocarray(ptr7, 5, 25);  // Still 125 bytes, but different calc
        if (ptr8) {
            int data_ok = 1;
            unsigned char *bytes = (unsigned char*)ptr8;
            for (int i = 0; i < 100; i++) {
                if (bytes[i] != 0x55) {
                    data_ok = 0;
                    break;
                }
            }
            test_assert(data_ok, "reallocarray() preserves data across different calculations");
            free(ptr8);
        } else {
            free(ptr7);
            test_assert(0, "reallocarray() recalculation failed");
        }
    }
}

void test_integration_extra() {
    printf(CYAN "\n=== Extra functions integration tests ===" NC "\n");
    
    // Test 1: Mix reallocarray with malloc_usable_size
    void *ptr1 = reallocarray(NULL, 20, 30);  // 600 bytes
    if (ptr1) {
        size_t usable1 = malloc_usable_size(ptr1);
        test_assert(usable1 >= 600, "Integration: reallocarray + malloc_usable_size");
        
        void *ptr2 = reallocarray(ptr1, 30, 40);  // 1200 bytes
        if (ptr2) {
            size_t usable2 = malloc_usable_size(ptr2);
            test_assert(usable2 >= 1200, "Integration: reallocarray expansion + malloc_usable_size");
            free(ptr2);
        } else {
            free(ptr1);
        }
    }
    
    // Test 2: Chain of reallocarray operations
    void *ptr = reallocarray(NULL, 1, 100);
    int chain_success = 1;
    
    if (ptr) {
        memset(ptr, 0x66, 100);
        
        // Chain several reallocarray calls
        for (int i = 2; i <= 10 && ptr; i++) {
            void *new_ptr = reallocarray(ptr, i, 100);
            if (new_ptr) {
                // Verify first 100 bytes are preserved
                unsigned char *bytes = (unsigned char*)new_ptr;
                for (int j = 0; j < 100; j++) {
                    if (bytes[j] != 0x66) {
                        chain_success = 0;
                        break;
                    }
                }
                ptr = new_ptr;
                
                // Fill new area
                memset((char*)ptr + 100, 0x77, (i - 1) * 100);
            } else {
                chain_success = 0;
                break;
            }
        }
        
        if (ptr) free(ptr);
    } else {
        chain_success = 0;
    }
    
    test_assert(chain_success, "Integration: chain of reallocarray operations");
    
    // Test 3: Stress test - many reallocarray operations
    void *stress_ptr = reallocarray(NULL, 1, 1);
    int stress_success = 1;
    
    for (int i = 1; i <= 100 && stress_ptr; i++) {
        void *new_ptr = reallocarray(stress_ptr, i, i);
        if (new_ptr) {
            stress_ptr = new_ptr;
            size_t usable = malloc_usable_size(stress_ptr);
            if (usable < (size_t)(i * i)) {
                stress_success = 0;
                break;
            }
        } else {
            stress_success = 0;
            break;
        }
    }
    
    if (stress_ptr) free(stress_ptr);
    test_assert(stress_success, "Integration: reallocarray stress test");
    
    // Test 4: malloc_usable_size with various allocation functions
    void *malloc_ptr = malloc(200);
    void *calloc_ptr = calloc(50, 4);
    void *realloc_ptr = realloc(NULL, 300);
    void *reallocarray_ptr = reallocarray(NULL, 25, 8);
    
    int mixed_usable_success = 1;
    
    if (malloc_ptr && malloc_usable_size(malloc_ptr) < 200) mixed_usable_success = 0;
    if (calloc_ptr && malloc_usable_size(calloc_ptr) < 200) mixed_usable_success = 0;
    if (realloc_ptr && malloc_usable_size(realloc_ptr) < 300) mixed_usable_success = 0;
    if (reallocarray_ptr && malloc_usable_size(reallocarray_ptr) < 200) mixed_usable_success = 0;
    
    test_assert(mixed_usable_success, "Integration: malloc_usable_size with all allocation types");
    
    if (malloc_ptr) free(malloc_ptr);
    if (calloc_ptr) free(calloc_ptr);
    if (realloc_ptr) free(realloc_ptr);
    if (reallocarray_ptr) free(reallocarray_ptr);
    
    // Test 5: PERTURB integration with aligned_alloc
    void *aligned_ptr = aligned_alloc(64, 128);  // 128 is multiple of 64
    if (aligned_ptr) {
        // Check alignment
        int is_aligned = ((uintptr_t)aligned_ptr % 64) == 0;
        test_assert(is_aligned, "Integration: aligned_alloc() alignment");
        
        // Check PERTURB initialization
        int perturb_ok = 1;
        unsigned char *bytes = (unsigned char*)aligned_ptr;
        for (int i = 0; i < 128; i++) {
            if (bytes[i] != 42) {
                perturb_ok = 0;
                break;
            }
        }
        test_assert(perturb_ok, "Integration: aligned_alloc() PERTURB initialization");
        
        // Check that malloc_usable_size works with aligned_alloc
        size_t usable = malloc_usable_size(aligned_ptr);
        test_assert(usable >= 128, "Integration: malloc_usable_size with aligned_alloc");

        free(aligned_ptr);
    }
}

int main() {
    test_reallocarray();
    test_malloc_usable_size();
    test_edge_cases();
    test_integration_extra();
}
