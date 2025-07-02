/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_stress.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 18:14:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/02 23:39:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>

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

double get_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

void test_massive_allocations() {
    printf(CYAN "\n=== Testing massive allocations ===" NC "\n");
    
    // Test 1: Many small allocations
    const int num_small = 10000;
    void **small_ptrs = malloc(num_small * sizeof(void*));
    int small_success = 1;
    
    if (small_ptrs) {
        for (int i = 0; i < num_small; i++) {
            small_ptrs[i] = malloc(i % 100 + 1);
            if (!small_ptrs[i]) {
                small_success = 0;
                break;
            }
        }
        
        test_assert(small_success, "10.000 small allocations");
        
        // Free them all
        for (int i = 0; i < num_small; i++) {
            if (small_ptrs[i]) free(small_ptrs[i]);
        }
        free(small_ptrs);
    } else {
        test_assert(0, "Failed to allocate pointer array");
    }
    
    // Test 2: Many medium allocations
    const int num_medium = 1000;
    void **medium_ptrs = malloc(num_medium * sizeof(void*));
    int medium_success = 1;
    
    if (medium_ptrs) {
        for (int i = 0; i < num_medium; i++) {
            medium_ptrs[i] = malloc(1024 + i);
            if (!medium_ptrs[i]) {
                medium_success = 0;
                break;
            }
        }
        
        test_assert(medium_success, "1.000 medium allocations (1KB+)");
        
        for (int i = 0; i < num_medium; i++) {
            if (medium_ptrs[i]) free(medium_ptrs[i]);
        }
        free(medium_ptrs);
    }
    
    // Test 3: Large allocations
    const int num_large = 100;
    void **large_ptrs = malloc(num_large * sizeof(void*));
    int large_success = 1;
    
    if (large_ptrs) {
        for (int i = 0; i < num_large; i++) {
            large_ptrs[i] = malloc(1024 * 1024 + i * 1024);  // 1MB+
            if (!large_ptrs[i]) {
                large_success = 0;
                break;
            }
        }
        
        test_assert(large_success, "100 large allocations (1MB+)");
        
        for (int i = 0; i < num_large; i++) {
            if (large_ptrs[i]) free(large_ptrs[i]);
        }
        free(large_ptrs);
    }
}

void test_fragmentation() {
    printf(CYAN "\n=== Testing fragmentation handling ===" NC "\n");
    
    // Test 1: Create fragmentation then try to allocate
    const int frag_count = 1000;
    void **frag_ptrs = malloc(frag_count * sizeof(void*));
    
    if (frag_ptrs) {
        // Allocate many blocks
        for (int i = 0; i < frag_count; i++) {
            frag_ptrs[i] = malloc(100);
        }
        
        // Free every other block to create fragmentation
        for (int i = 1; i < frag_count; i += 2) {
            free(frag_ptrs[i]);
            frag_ptrs[i] = NULL;
        }
        
        // Try to allocate in the gaps
        int gaps_filled = 0;
        for (int i = 1; i < frag_count; i += 2) {
            frag_ptrs[i] = malloc(50);  // Smaller than original
            if (frag_ptrs[i]) gaps_filled++;
        }
        
        test_assert(gaps_filled > frag_count / 4, "Fragmentation handling (filled gaps)");
        
        // Clean up
        for (int i = 0; i < frag_count; i++) {
            if (frag_ptrs[i]) free(frag_ptrs[i]);
        }
        free(frag_ptrs);
    }
    
    // Test 2: Alternating sizes
    void *alt_ptrs[200];
    int alt_success = 1;
    
    for (int i = 0; i < 200; i++) {
        if (i % 2 == 0) {
            alt_ptrs[i] = malloc(32);
        } else {
            alt_ptrs[i] = malloc(1024);
        }
        if (!alt_ptrs[i]) alt_success = 0;
    }
    
    test_assert(alt_success, "Alternating small/large allocations");
    
    for (int i = 0; i < 200; i++) {
        if (alt_ptrs[i]) free(alt_ptrs[i]);
    }
}

void test_realloc_patterns() {
    printf(CYAN "\n=== Testing realloc patterns ===" NC "\n");
    
    // Test 1: Growing pattern
    void *ptr = malloc(100);
    int grow_success = 1;
    
    if (ptr) {
        memset(ptr, 0x99, 100);
        
        for (int size = 200; size <= 10000; size += 100) {
            void *new_ptr = realloc(ptr, size);
            if (new_ptr) {
                // Check if original data is preserved
                unsigned char *bytes = (unsigned char*)new_ptr;
                int data_ok = 1;
                for (int i = 0; i < 100; i++) {
                    if (bytes[i] != 0x99) {
                        data_ok = 0;
                        break;
                    }
                }
                if (!data_ok) {
                    grow_success = 0;
                    free(new_ptr);
                    break;
                }
                ptr = new_ptr;
            } else {
                grow_success = 0;
                free(ptr);
                break;
            }
        }
        
        if (ptr) free(ptr);
    } else {
        grow_success = 0;
    }
    
    test_assert(grow_success, "Realloc growing pattern");
    
    // Test 2: Shrinking pattern
    ptr = malloc(10000);
    int shrink_success = 1;
    
    if (ptr) {
        memset(ptr, 0xAA, 10000);
        
        for (int size = 9000; size >= 100; size -= 100) {
            void *new_ptr = realloc(ptr, size);
            if (new_ptr) {
                // Check if data is preserved up to new size
                unsigned char *bytes = (unsigned char*)new_ptr;
                int data_ok = 1;
                for (int i = 0; i < size && i < 10000; i++) {
                    if (bytes[i] != 0xAA) {
                        data_ok = 0;
                        break;
                    }
                }
                if (!data_ok) {
                    shrink_success = 0;
                    free(new_ptr);
                    break;
                }
                ptr = new_ptr;
            } else {
                shrink_success = 0;
                if (ptr) free(ptr);
                break;
            }
        }
        
        if (ptr) free(ptr);
    } else {
        shrink_success = 0;
    }
    
    test_assert(shrink_success, "Realloc shrinking pattern");
}

void test_memory_patterns() {
    printf(CYAN "\n=== Testing memory access patterns ===" NC "\n");
    
    // Test 1: Write patterns to detect corruption
    void *ptr1 = malloc(4096);
    if (ptr1) {
        // Fill with pattern
        uint32_t *words = (uint32_t*)ptr1;
        for (int i = 0; i < 1024; i++) {
            words[i] = 0xDEADBEEF;
        }
        
        // Verify pattern
        int pattern_ok = 1;
        for (int i = 0; i < 1024; i++) {
            if (words[i] != 0xDEADBEEF) {
                pattern_ok = 0;
                break;
            }
        }
        
        test_assert(pattern_ok, "Memory pattern integrity");
        free(ptr1);
    }
    
    // Test 2: Random access pattern
    void *ptr2 = malloc(8192);
    if (ptr2) {
        unsigned char *bytes = (unsigned char*)ptr2;
        
        // Write random pattern
        for (int i = 0; i < 8192; i++) {
            bytes[i] = (unsigned char)(i ^ 0x55);
        }
        
        // Verify with random access
        int random_ok = 1;
        for (int test = 0; test < 1000; test++) {
            int idx = rand() % 8192;
            if (bytes[idx] != (unsigned char)(idx ^ 0x55)) {
                random_ok = 0;
                break;
            }
        }
        
        test_assert(random_ok, "Random access pattern");
        free(ptr2);
    }
    
    // Test 3: Boundary writes
    void *ptr3 = malloc(1000);
    if (ptr3) {
        unsigned char *bytes = (unsigned char*)ptr3;
        
        // Write to boundaries
        bytes[0] = 0xFF;
        bytes[999] = 0xFF;
        bytes[500] = 0xFF;
        
        int boundary_ok = (bytes[0] == 0xFF && bytes[999] == 0xFF && bytes[500] == 0xFF);
        test_assert(boundary_ok, "Boundary access");
        free(ptr3);
    }
}

void test_performance() {
    printf(CYAN "\n=== Performance tests ===" NC "\n");
    
    struct timeval start, end;
    
    // Test 1: Allocation speed
    gettimeofday(&start, NULL);
    
    const int perf_count = 10000;
    void **perf_ptrs = malloc(perf_count * sizeof(void*));
    
    if (perf_ptrs) {
        for (int i = 0; i < perf_count; i++) {
            perf_ptrs[i] = malloc(100);
        }
        
        gettimeofday(&end, NULL);
        double alloc_time = get_time_diff(start, end);
        
        // printf("    Allocation time for %d blocks: %.3f seconds\n", perf_count, alloc_time);
        test_assert(alloc_time < 1.0, "Allocation performance (10.000 blocks in less than 1 second)");
        
        // Test 2: Free speed
        gettimeofday(&start, NULL);
        
        for (int i = 0; i < perf_count; i++) {
            if (perf_ptrs[i]) free(perf_ptrs[i]);
        }
        
        gettimeofday(&end, NULL);
        double free_time = get_time_diff(start, end);
        
        // printf("    Free time for %d blocks: %.3f seconds\n", perf_count, free_time);
        test_assert(free_time < 1.0, "Free performance (10.000 blocks in less than 1 second)");
        
        free(perf_ptrs);
    }
    
    // Test 3: Realloc performance
    gettimeofday(&start, NULL);
    
    void *ptr = malloc(100);
    for (int i = 1; i <= 1000; i++) {
        ptr = realloc(ptr, i * 100);
        if (!ptr) break;
    }
    
    gettimeofday(&end, NULL);
    double realloc_time = get_time_diff(start, end);
    
    // printf("    Realloc time for 1000 operations: %.3f seconds\n", realloc_time);
    test_assert(realloc_time < 1.0, "Realloc performance (1.000 operations in less than 1 second)");
    
    if (ptr) free(ptr);
}

void test_error_conditions() {
    printf(CYAN "\n=== Testing error conditions ===" NC "\n");
  
    // Test 1: Invalid realloc
    void *invalid_ptr = (void*)0xDEADBEEF;
    void *result = realloc(invalid_ptr, 100);
    test_assert(result == NULL, "Realloc with invalid pointer fails gracefully");
    
    // Test 2: Extremely large allocation
    void *huge = malloc(SIZE_MAX - 1);
    test_assert(huge == NULL, "Extremely large malloc fails gracefully");
    
    // Test 3: Negative size (when cast to size_t becomes huge)
    void *neg = malloc((size_t)-1);
    test_assert(neg == NULL, "Negative size malloc fails gracefully");
}

void *thread_alloc_test(void *arg) {
    int thread_id = *(int*)arg;
    const int allocs_per_thread = 1000;
    void *ptrs[allocs_per_thread];
    
    // Allocate
    for (int i = 0; i < allocs_per_thread; i++) {
        ptrs[i] = malloc(100 + (thread_id * 10) + i);
        if (ptrs[i]) {
            memset(ptrs[i], thread_id & 0xFF, 100);
        }
    }
    
    // Free
    for (int i = 0; i < allocs_per_thread; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }
    
    return NULL;
}

void test_threading() {
    printf(CYAN "\n=== Testing thread safety ===" NC "\n");
    
    const int num_threads = 4;
    pthread_t threads[num_threads];
    int thread_ids[num_threads];
    
    // Create threads
    int thread_success = 1;
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_alloc_test, &thread_ids[i]) != 0) {
            thread_success = 0;
            break;
        }
    }
    
    // Wait for threads
    if (thread_success) {
        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }
    }
    
    test_assert(thread_success, "Multi-threaded allocation test");
}

int main() {
    test_massive_allocations();
    test_fragmentation();
    test_realloc_patterns();
    test_memory_patterns();
    test_performance();
    test_error_conditions();
    test_threading();
}
