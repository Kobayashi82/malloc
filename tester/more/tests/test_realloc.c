#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../includes/ft_malloc.h"

void test_realloc()
{
    // Test 1: Reallocate NULL pointer
    void *ptr = realloc(NULL, 10);
    assert(ptr != NULL);
    memset(ptr, 0, 10);
    free(ptr);

    // Test 2: Reallocate to a larger size
    ptr = malloc(10);
    assert(ptr != NULL);
    memset(ptr, 'A', 10);
    ptr = realloc(ptr, 20);
    assert(ptr != NULL);
    assert(((char *)ptr)[0] == 'A');
    assert(((char *)ptr)[9] == 'A');
    assert(((char *)ptr)[10] != 'A');
    free(ptr);

    // Test 3: Reallocate to a smaller size
    ptr = malloc(20);
    assert(ptr != NULL);
    memset(ptr, 'B', 20);
    ptr = realloc(ptr, 10);
    assert(ptr != NULL);
    assert(((char *)ptr)[0] == 'B');
    assert(((char *)ptr)[9] == 'B');
    free(ptr);

    // Test 4: Reallocate with size zero (should free memory)
    ptr = malloc(10);
    assert(ptr != NULL);
    ptr = realloc(ptr, 0);
    assert(ptr == NULL);

    printf("All tests passed!\n");
}

int main()
{
    test_realloc();
    show_alloc_mem();
    return 0;
}
