#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "../includes/ft_malloc.h"

void test_zones()
{
    void *ptr = malloc(16);
    memset(ptr, 'A', 16);

    void *ptr2 = malloc(2);
    memset(ptr2, 'B', 2);

    void *ptr3 = malloc(9);
    memset(ptr3, 'C', 9);

    free(ptr2);

    void *ptr4 = malloc(20);
    memset(ptr4, 'D', 20);

    void *ptr5 = malloc(2); // should be placed where ptr2 was
    memset(ptr5, 'E', 2);

    void *ptr6 = malloc(24); // next tiny zone
    memset(ptr6, 'F', 24);

    void *ptr7 = malloc(19);
    memset(ptr7, 'G', 19);

    void *ptr8 = malloc(257); // small zone
    memset(ptr8, 'A', 257);

    void *ptr9 = malloc(300000); // large zone
    memset(ptr9, 'B', 300000);
}

void test_realloc()
{
    void *r1 = malloc(16);
    memset(r1, 'A', 16);
    printf("FIRST!\n");

    void *r2 = realloc(r1, 16); // same size
    assert(*((char *)r2) == 'A');
    assert(*((char *)r2 + 15) == 'A');

    void *r3 = realloc(r2, 17); // a byte bigger
    assert(*((char *)r3) == 'A');
    assert(*((char *)r3 + 15) == 'A');
    assert(*((char *)r3 + 16) != 'A');

    void *r4 = realloc(r3, 15); // a byte smaller
    assert(*((char *)r4) == 'A');
    assert(*((char *)r4 + 14) == 'A');

    void *r5 = realloc(r4, 2); // a lot smaller
    assert(*((char *)r4) == 'A');
    assert(*((char *)r4 + 1) == 'A');

    void *r6 = realloc(r5, 30); // a lot bigger
    assert(*((char *)r5) == 'A');
    assert(*((char *)r5 + 29) != 'A');

    printf("realloc passed!\n");
}

void test_coalescing()
{
    void *c1 = malloc(16);
    memset(c1, 'A', 16);

    void *c2 = malloc(16);
    memset(c2, 'B', 16);

    void *c3 = malloc(4);
    memset(c3, 'C', 4);

    void *c4 = malloc(8);
    memset(c4, 'D', 8);

    free(c2); // not coales

    free(c3); // coales with prev

    free(c4); // coales with prev and next

    free(c1); // coales with next

    // void *c5 = malloc(120); // fill first tiny zone
    // memset(c5, 'E', 120);

    // void *c6 = malloc(120); // fill second tiny zone
    // memset(c6, 'F', 120);

    // void *c7 = malloc(20); // start third tiny zone, so that 2nd has next and prev pointers set
    // memset(c7, 'G', 20);

    // free(c6); // not coalescing

    printf("coalescing passed!\n");
}

void test_big_allocations()
{
    void *b1 = malloc(1024 * 1024); // 1M;
    memset(b1, 'A', 1024 * 1024);

    void *b2 = malloc(1024 * 1024); // 1M;
    memset(b2, 'B', 1024 * 1024);

    free(b1);
    free(b2);

    void *b3 = malloc(1024 * 1024);
    memset(b3, 'C', 1024 * 1024);

    free(b3);
}

int main()
{
    test_coalescing();
    test_zones();
    test_realloc();
    test_big_allocations();

    return 0;
}
