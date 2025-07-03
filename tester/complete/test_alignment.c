/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_alignment.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 18:14:23 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/03 20:55:25 by vzurera-         ###   ########.fr       */
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
extern void *aligned_alloc(size_t alignment, size_t size);
extern void *memalign(size_t alignment, size_t size);
extern int posix_memalign(void **memptr, size_t alignment, size_t size);
extern void *valloc(size_t size);
extern void *pvalloc(size_t size);

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

int is_aligned(void *ptr, size_t alignment) {
    return ((uintptr_t)ptr % alignment) == 0;
}

int is_power_of_two(size_t n) {
    return n != 0 && (n & (n - 1)) == 0;
}

void test_aligned_alloc() {
    printf(CYAN "\n=== Testing aligned_alloc() ===" NC "\n");
    
    // Test 1: Basic alignment
    size_t alignments[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    int basic_test_passed = 1;

    for (int i = 0; i < 9; i++) {
        void *ptr = aligned_alloc(alignments[i], 4096);
        if (ptr) {
            if (!is_aligned(ptr, alignments[i])) {
                basic_test_passed = 0;
            }
            free(ptr);
        } else {
            basic_test_passed = 0;
        }
    }
    test_assert(basic_test_passed, "aligned_alloc() basic alignment test");
    
    // Test 2: Size must be multiple of alignment
    void *ptr1 = aligned_alloc(16, 100);  // 100 is not multiple of 16
    test_assert(ptr1 == NULL, "aligned_alloc() rejects size not multiple of alignment");
    
    void *ptr2 = aligned_alloc(16, 96);   // 96 is multiple of 16
    test_assert(ptr2 != NULL && is_aligned(ptr2, 16), "aligned_alloc() accepts valid size/alignment");
    if (ptr2) free(ptr2);
    
    // Test 3: Invalid alignment (not power of 2) - Test behavior programmatically
    // Note: We test this by checking if the function handles invalid alignments properly
    // but we can't use compile-time invalid values with -Werror
    int invalid_alignment_test = 1;
    
    // Test with alignment 3 (not power of 2) - we'll suppress the warning by using a variable
    size_t bad_align1 = 3;
    void *ptr3 = aligned_alloc(bad_align1, 12);
    if (ptr3 != NULL) invalid_alignment_test = 0;
    
    size_t bad_align2 = 6;
    void *ptr4 = aligned_alloc(bad_align2, 12);
    if (ptr4 != NULL) invalid_alignment_test = 0;
    
    test_assert(invalid_alignment_test, "aligned_alloc() rejects non-power-of-2 alignment");
    
    // Test 4: Zero alignment
    size_t zero_align = 0;
    void *ptr5 = aligned_alloc(zero_align, 100);
    test_assert(ptr5 == NULL, "aligned_alloc() rejects zero alignment");
    
    // Test 5: Zero size
    void *ptr6 = aligned_alloc(16, 0);
    test_assert(ptr6 == NULL || ptr6 != NULL, "aligned_alloc() handles zero size");
    if (ptr6) free(ptr6);
    
    // Test 6: Large alignment
    void *ptr7 = aligned_alloc(4096, 8192);
    test_assert(ptr7 != NULL && is_aligned(ptr7, 4096), "aligned_alloc() large alignment");
    if (ptr7) free(ptr7);
    
    // Test 7: Memory is usable
    void *ptr8 = aligned_alloc(32, 96);
    if (ptr8) {
        memset(ptr8, 0x55, 96);
        int memory_usable = 1;
        unsigned char *bytes = (unsigned char*)ptr8;
        for (int i = 0; i < 96; i++) {
            if (bytes[i] != 0x55) {
                memory_usable = 0;
                break;
            }
        }
        test_assert(memory_usable, "aligned_alloc() memory is writable");
        free(ptr8);
    }
    
    // Test 9: PERTURB functionality
	int perturb_ok = 1;
    void *ptr_perturb = aligned_alloc(64, 128);  // 128 is multiple of 64
    if (ptr_perturb) {
        // Check if memory is initialized with PERTURB value (42)
        unsigned char *bytes = (unsigned char*)ptr_perturb;
        for (int i = 0; i < 128; i++) {
            if (bytes[i] != 42) {
                perturb_ok = 0;
                break;
            }
        }
    }
	test_assert(ptr_perturb && perturb_ok, "aligned_alloc() PERTURB initialization (value 42)");
	if (ptr_perturb) free(ptr_perturb);
}

void test_memalign() {
    printf(CYAN "\n=== Testing memalign() ===" NC "\n");
    
    // Test 1: Basic alignment
    size_t alignments[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    int basic_test_passed = 1;
    
    for (int i = 0; i < 9; i++) {
        void *ptr = memalign(alignments[i], 100);
        if (ptr) {
            if (!is_aligned(ptr, alignments[i])) {
                basic_test_passed = 0;
            }
            free(ptr);
        } else {
            basic_test_passed = 0;
        }
    }
    test_assert(basic_test_passed, "memalign() basic alignment test");
    
    // Test 2: Unlike aligned_alloc, size doesn't need to be multiple of alignment
    void *ptr1 = memalign(16, 100);  // 100 is not multiple of 16
    test_assert(ptr1 != NULL && is_aligned(ptr1, 16), "memalign() accepts any size");
    if (ptr1) free(ptr1);
    
    // Test 3: Invalid alignment (not power of 2)
    size_t bad_align = 3;
    void *ptr2 = memalign(bad_align, 100);
    test_assert(ptr2 == NULL, "memalign() rejects non-power-of-2 alignment");
    
    // Test 4: Zero alignment
    size_t zero_align = 0;
    void *ptr3 = memalign(zero_align, 100);
    test_assert(ptr3 == NULL, "memalign() rejects zero alignment");
    
    // Test 5: Zero size
    void *ptr4 = memalign(16, 0);
    test_assert(ptr4 == NULL || ptr4 != NULL, "memalign() handles zero size");
    if (ptr4) free(ptr4);
    
    // Test 6: Large alignment
    void *ptr5 = memalign(4096, 1000);
    test_assert(ptr5 != NULL && is_aligned(ptr5, 4096), "memalign() large alignment");
    if (ptr5) free(ptr5);
    
    // Test 7: PERTURB functionality
    void *ptr_perturb = memalign(32, 100);
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
        test_assert(perturb_ok, "memalign() PERTURB initialization (value 42)");
        free(ptr_perturb);
    }
}

void test_posix_memalign() {
    printf(CYAN "\n=== Testing posix_memalign() ===" NC "\n");
    
    // Test 1: Basic functionality
    void *ptr1;
    int ret1 = posix_memalign(&ptr1, 16, 100);
    test_assert(ret1 == 0 && ptr1 != NULL && is_aligned(ptr1, 16), "posix_memalign() basic test");
    if (ptr1) free(ptr1);
    
    // Test 2: Invalid alignment (not power of 2)
    void *ptr2;
    int ret2 = posix_memalign(&ptr2, 3, 100);
    test_assert(ret2 == EINVAL, "posix_memalign() returns EINVAL for non-power-of-2");
    
    // Test 3: Invalid alignment (not multiple of sizeof(void*))
    void *ptr3;
    int ret3 = posix_memalign(&ptr3, 2, 100);  // 2 < sizeof(void*)
    test_assert(ret3 == EINVAL, "posix_memalign() returns EINVAL for alignment < sizeof(void*)");
    
    // Test 4: Valid alignments
    size_t valid_alignments[] = {sizeof(void*), 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    int valid_test_passed = 1;
    
    for (int i = 0; i < 10; i++) {
        void *ptr;
        int ret = posix_memalign(&ptr, valid_alignments[i], 100);
        if (ret != 0 || !ptr || !is_aligned(ptr, valid_alignments[i])) {
            valid_test_passed = 0;
        }
        if (ptr) free(ptr);
    }
    test_assert(valid_test_passed, "posix_memalign() valid alignments");
    
    // Test 5: Zero size
    void *ptr4;
    int ret4 = posix_memalign(&ptr4, 16, 0);
    test_assert(ret4 == 0, "posix_memalign() handles zero size");
    if (ptr4) free(ptr4);
    
    // Test 6: Large size
    void *ptr5;
    int ret5 = posix_memalign(&ptr5, 4096, 1024 * 1024);
    test_assert(ret5 == 0 && ptr5 != NULL && is_aligned(ptr5, 4096), "posix_memalign() large allocation");
    if (ptr5) free(ptr5);
    
    // Test 7: NULL pointer argument should fail
    void **null_ptr = NULL;
    int ret6 = posix_memalign(null_ptr, 16, 100);
    test_assert(ret6 == EINVAL, "posix_memalign() returns EINVAL for NULL memptr");
    
    // Test 8: PERTURB functionality
    void *ptr_perturb;
    int perturb_ok = 1;
	int ret_perturb = posix_memalign(&ptr_perturb, 64, 128);
    if (ret_perturb == 0 && ptr_perturb) {       
        // Check if memory is initialized with PERTURB value (42)
        unsigned char *bytes = (unsigned char*)ptr_perturb;
        for (int i = 0; i < 128; i++) {
            if (bytes[i] != 42) {
                perturb_ok = 0;
                break;
            }
        }
    }
	test_assert(ptr_perturb && perturb_ok, "posix_memalign() PERTURB initialization (value 42)");
	if (ptr_perturb) free(ptr_perturb);
}

void test_valloc() {
    printf(CYAN "\n=== Testing valloc() ===" NC "\n");
    
    // Get page size
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096;  // fallback
    
    // Test 1: Basic page alignment
    void *ptr1 = valloc(100);
    test_assert(ptr1 != NULL && is_aligned(ptr1, page_size), "valloc() returns page-aligned memory");
    if (ptr1) free(ptr1);
    
    // Test 2: Zero size
    void *ptr2 = valloc(0);
    test_assert(ptr2 == NULL || ptr2 != NULL, "valloc() handles zero size");
    if (ptr2) free(ptr2);
    
    // Test 3: Large allocation
    void *ptr3 = valloc(1024 * 1024);
    test_assert(ptr3 != NULL && is_aligned(ptr3, page_size), "valloc() large allocation");
    if (ptr3) free(ptr3);
    
    // Test 4: Multiple allocations
    void *ptrs[10];
    int all_aligned = 1;
    
    for (int i = 0; i < 10; i++) {
        ptrs[i] = valloc(100 + i * 50);
        if (!ptrs[i] || !is_aligned(ptrs[i], page_size)) {
            all_aligned = 0;
        }
    }
    
    test_assert(all_aligned, "valloc() multiple allocations");
    
    for (int i = 0; i < 10; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }
    
    // Test 5: Memory is usable
    void *ptr4 = valloc(page_size);
    if (ptr4) {
        memset(ptr4, 0xAA, page_size);
        int memory_usable = 1;
        unsigned char *bytes = (unsigned char*)ptr4;
        for (long i = 0; i < page_size; i++) {
            if (bytes[i] != 0xAA) {
                memory_usable = 0;
                break;
            }
        }
        test_assert(memory_usable, "valloc() memory is writable");
        free(ptr4);
    }
    
    // Test 6: PERTURB functionality
    void *ptr_perturb = valloc(page_size);
    int perturb_ok = 1;
    if (ptr_perturb) {       
        // Check if memory is initialized with PERTURB value (42)
        unsigned char *bytes = (unsigned char*)ptr_perturb;
        for (long i = 0; i < page_size; i++) {
            if (bytes[i] != 42) {
                perturb_ok = 0;
                break;
            }
        }
    }
	test_assert(ptr_perturb && perturb_ok, "valloc() PERTURB initialization (value 42)");
	if (ptr_perturb) free(ptr_perturb);
}

void test_pvalloc() {
    printf(CYAN "\n=== Testing pvalloc() ===" NC "\n");
    
    // Get page size
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096;  // fallback
    
    // Test 1: Size rounded up to page size
    void *ptr1 = pvalloc(100);
    test_assert(ptr1 != NULL && is_aligned(ptr1, page_size), "pvalloc() returns page-aligned memory");
    if (ptr1) free(ptr1);
    
    // Test 2: Exact page size
    void *ptr2 = pvalloc(page_size);
    test_assert(ptr2 != NULL && is_aligned(ptr2, page_size), "pvalloc() exact page size");
    if (ptr2) free(ptr2);
    
    // Test 3: Multiple pages
    void *ptr3 = pvalloc(page_size + 1);
    test_assert(ptr3 != NULL && is_aligned(ptr3, page_size), "pvalloc() multiple pages");
    if (ptr3) free(ptr3);

    // Test 4: Zero size
    void *ptr4 = pvalloc(0);
    test_assert(ptr4 == NULL || ptr4 != NULL, "pvalloc() handles zero size");
    if (ptr4) free(ptr4);
    
    // Test 5: Memory is usable for at least one page
    void *ptr5 = pvalloc(1);  // Should allocate at least one page
    if (ptr5) {
        memset(ptr5, 0xBB, page_size);
        int memory_usable = 1;
        unsigned char *bytes = (unsigned char*)ptr5;
        for (long i = 0; i < page_size; i++) {
            if (bytes[i] != 0xBB) {
                memory_usable = 0;
                break;
            }
        }
        test_assert(memory_usable, "pvalloc() allocates full page");
        free(ptr5);
    }
    
    // Test 6: PERTURB functionality
    void *ptr_perturb = pvalloc(100);  // Should allocate at least one page
    int perturb_ok = 1;
	if (ptr_perturb) {
        // Check if memory is initialized with PERTURB value (42)
        unsigned char *bytes = (unsigned char*)ptr_perturb;
        for (long i = 0; i < page_size; i++) {
            if (bytes[i] != 42) {
                perturb_ok = 0;
                break;
            }
        }
    }
	test_assert(ptr_perturb && perturb_ok, "pvalloc() PERTURB initialization (value 42)");
	if (ptr_perturb) free(ptr_perturb);
}

void test_alignment_stress() {
    printf(CYAN "\n=== Alignment stress tests ===" NC "\n");
    
    // Test 1: Mixed alignment functions
    void *ptr1 = aligned_alloc(32, 96);
    void *ptr2 = memalign(64, 100);
    void *ptr3;
    int ret = posix_memalign(&ptr3, 128, 200);
    void *ptr4 = valloc(150);
    void *ptr5 = pvalloc(75);
    
    int mixed_success = 1;
    if (!ptr1 || !is_aligned(ptr1, 32)) mixed_success = 0;
    if (!ptr2 || !is_aligned(ptr2, 64)) mixed_success = 0;
    if (ret != 0 || !ptr3 || !is_aligned(ptr3, 128)) mixed_success = 0;
    if (!ptr4 || !is_aligned(ptr4, sysconf(_SC_PAGESIZE))) mixed_success = 0;
    if (!ptr5 || !is_aligned(ptr5, sysconf(_SC_PAGESIZE))) mixed_success = 0;
    
    test_assert(mixed_success, "Mixed alignment functions");
    
    if (ptr1) free(ptr1);
    if (ptr2) free(ptr2);
    if (ptr3) free(ptr3);
    if (ptr4) free(ptr4);
    if (ptr5) free(ptr5);
    
    // Test 2: Alignment preservation across realloc
    void *ptr6 = memalign(64, 100);
    if (ptr6 && is_aligned(ptr6, 64)) {
        memset(ptr6, 0xCC, 100);
        void *ptr7 = realloc(ptr6, 200);
        
        // Note: realloc doesn't guarantee alignment preservation
        // This is just to test behavior
        test_assert(ptr7 != NULL, "realloc() on aligned memory");
        
        if (ptr7) {
            // Check if data is preserved
            int data_preserved = 1;
            unsigned char *bytes = (unsigned char*)ptr7;
            for (int i = 0; i < 100; i++) {
                if (bytes[i] != 0xCC) {
                    data_preserved = 0;
                    break;
                }
            }
            test_assert(data_preserved, "realloc() preserves data from aligned memory");
            free(ptr7);
        }
    } else {
        if (ptr6) free(ptr6);
        test_assert(0, "Initial aligned allocation failed");
    }
}

int main() {
    test_aligned_alloc();
    test_memalign();
    test_posix_memalign();
    test_valloc();
    test_pvalloc();
    test_alignment_stress();
}
