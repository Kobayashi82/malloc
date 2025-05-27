#include <stdio.h>
#include <assert.h>
#include <string.h> // For memset
#include "../includes/ft_malloc.h"

void test_malloc()
{
    printf("Running tests for custom malloc...\n");

    // Test 1: Allocate memory of a small size
    void *ptr = malloc(10);
    assert(ptr != NULL);
    memset(ptr, 0, 10);
    for (int i = 0; i < 10; i++)
    {
        ((char *)ptr)[i] = (char)(i + 1);
        assert(((char *)ptr)[i] == (char)(i + 1));
    }
    free(ptr);

    // Test 2: Allocate memory of a large size
    ptr = malloc(1024 * 1024);
    assert(ptr != NULL);
    memset(ptr, 0, 1024 * 1024);
    for (int i = 0; i < 1024 * 1024; i++)
    {
        ((char *)ptr)[i] = (char)(i % 256);
        assert(((char *)ptr)[i] == (char)(i % 256));
    }
    free(ptr);

    // Test 3: Allocate zero bytes
    ptr = malloc(0);
    assert(ptr == NULL);
    if (ptr)
        free(ptr);

    // Test 4: Multiple allocations and freeing
    void *ptr1 = malloc(50);
    void *ptr2 = malloc(100);
    void *ptr3 = malloc(200);
    assert(ptr1 != NULL && ptr2 != NULL && ptr3 != NULL);

    memset(ptr1, 'A', 50);
    memset(ptr2, 'B', 100);
    memset(ptr3, 'C', 200);

    for (int i = 0; i < 50; i++)
    {
        assert(((char *)ptr1)[i] == 'A');
    }
    for (int i = 0; i < 100; i++)
    {
        assert(((char *)ptr2)[i] == 'B');
    }
    for (int i = 0; i < 200; i++)
    {
        assert(((char *)ptr3)[i] == 'C');
    }

    free(ptr1);
    free(ptr2);
    free(ptr3);

    // Test 5: Stress test with many small allocations
    const int num_allocations = 50000;
    void *allocations[num_allocations];

    for (int i = 0; i < num_allocations; i++)
    {
        allocations[i] = malloc(16);
        assert(allocations[i] != NULL);

        for (int j = 0; j < 16; j++)
        {
            ((char *)allocations[i])[j] = (char)(j + i % 256);
            assert(((char *)allocations[i])[j] == (char)(j + i % 256));
        }
    }

    for (int i = 0; i < num_allocations; i++)
        free(allocations[i]);

    printf("All tests passed!\n");
}

int main()
{
    test_malloc();
    show_alloc_mem();
    return 0;
}
