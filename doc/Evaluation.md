# Malloc Project Evaluation Guide

## Preliminaries

### Preliminary Tests

First check the following items:
- There is indeed a rendering (in the git repository)
- Valid author file
- The Makefile is present and has the requested rules
- No fault of norm, the Norminette being authentic
- No cheating (unauthorized functions...)
- 2 globals are allowed: one to manage allocations, and one to manage thread-safe

**If an element does not conform to the subject, the notation stops there.**

### Compilation of the Library

Check that the compilation of the library generates the files requested in the subject, by modifying HOSTTYPE:

```bash
$> export HOSTTYPE=Testing
$> ls -l libft_malloc_Testing.so libft_malloc.so
$> ls -l libft_malloc.so
libft_malloc.so -> libft_malloc_Testing.so
```

The Makefile uses HOSTTYPE to define the name of the library (`libft_malloc_$HOSTTYPE.so`) and creates a symbolic link `libft_malloc.so` pointing to `libft_malloc_$HOSTTYPE.so`.

**If this is not the case, the defense stops.**

### Export of Functions

Check with `nm` that the library exports the `malloc`, `free`, `realloc` and `show_alloc_mem` functions:

```bash
$> nm libft_malloc.so
0000000000000000 T _free
0000000000000000 T _malloc
0000000000000000 T _realloc
0000000000000000 T _show_alloc_mem
                 U _munmap
                 U _getpagesize
                 U _write
                 U _dyld_stub_binder
```

The functions exported by the library are marked with a `T`, those used with a `U`.

**If the functions are not exported, the defense stops.**

## Functionality Tests

### Launch Script Setup

Create a launch script that modifies environment variables for testing:

```bash
$> cat run.sh
#!/bin/sh
export DYLD_LIBRARY_PATH=.
export DYLD_INSERT_LIBRARIES="libft_malloc.so"
export DYLD_FORCE_FLAT_NAMESPACE=1
$*
```

Make it executable: `chmod +x run.sh`

### Malloc Test

#### Baseline Test (test0.c)
```c
#include <stdlib.h>

int main()
{
    int i;
    char *addr;
    
    i = 0;
    while (i < 1024)
        i++;
    return (0);
}
```

```bash
$> cc -o test0 test0.c
$> /usr/bin/time -l ./test0
0.00 real         0.00 user         0.00 sys
491520  maximum resident set size
```

#### Malloc Test (test1.c)
```c
#include <stdlib.h>

int main()
{
    int i;
    char *addr;
    
    i = 0;
    while (i < 1024)
    {
        addr = (char*)malloc(1024);
        addr[0] = 42;
        i++;
    }
    return (0);
}
```

```bash
$> cc -o test1 test1.c
$> /usr/bin/time -l ./test1
0.00 real         0.00 user         0.00 sys
1544192  maximum resident set size
396 page reclaims
```

#### Testing with Custom Library

```bash
$> ./run.sh /usr/bin/time -l ./test0
0.01 real         0.00 user         0.00 sys
708608  maximum resident set size
214 page reclaims

$> ./run.sh /usr/bin/time -l ./test1
0.00 real         0.00 user         0.00 sys
4902912  maximum resident set size
1238 page reclaims
```

#### Memory Usage Evaluation

Ask the student about TINY and SMALL zone limits to determine if 1024-byte blocks are TINY, SMALL, or LARGE.

**Rating based on pages used:**
- Less than 255 pages: insufficient reserved memory - **0 points**
- 1024+ pages: works but consumes minimum page per allocation - **1 point**
- 512-1023 pages: works but overhead too large - **2 points**
- 342-511 pages: works but overhead very important - **3 points**
- 291-341 pages: works but overhead important - **4 points**
- 255-290 pages: works with reasonable overhead - **5 points**

### Pre-allocated Areas

Check in source code that pre-allocated areas can store at least 100 times the max size for each area type.

Also verify that area sizes are multiples of `getpagesize()`.

**If either point is missing, grade as "No".**

### Free Test (test2.c)

```c
#include <stdlib.h>

int main()
{
    int i;
    char *addr;
    
    i = 0;
    while (i < 1024)
    {
        addr = (char*)malloc(1024);
        addr[0] = 42;
        free(addr);
        i++;
    }
    return (0);
}
```

```bash
$> cc -o test2 test2.c
$> ./run.sh /usr/bin/time -l ./test2
```

Compare "page reclaims" with test0 and test1. If equal or more than test1, free doesn't work.

**Does free work?** (fewer "page reclaims" than test1)

**Quality of free:** test2 has maximum 3 additional "page reclaims" compared to test0?

### Realloc Test (test3.c)

```c
#include <strings.h>
#include <stdlib.h>
#define M (1024*1024)

void print(char *s)
{
    write(1, s, strlen(s));
}

int main()
{
    char *addr1;
    char *addr3;
    
    addr1 = (char*)malloc(16*M);
    strcpy(addr1, "Hello\n");
    print(addr1);
    addr3 = (char*)realloc(addr1, 128*M);
    addr3[127*M] = 42;
    print(addr3);
    return (0);
}
```

```bash
$> cc -o test3 test3.c
$> ./run.sh ./test3
Hello
Hello
```

**Does it work as expected?**

#### Enhanced Realloc Test

Modify test3.c main function:

```c
int main()
{
    char *addr1;
    char *addr2;
    char *addr3;
    
    addr1 = (char*)malloc(16*M);
    strcpy(addr1, "Hello\n");
    print(addr1);
    addr2 = (char*)malloc(16*M);
    addr3 = (char*)realloc(addr1, 128*M);
    addr3[127*M] = 42;
    print(addr3);
    return (0);
}
```

### Error Handling (test4.c)

```c
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void print(char *s)
{
    write(1, s, strlen(s));
}

int main()
{
    char *addr;
    
    addr = malloc(16);
    free(NULL);
    free((void *)addr + 5);
    if (realloc((void *)addr + 5, 10) == NULL)
        print("Hello\n");
}
```

```bash
$> cc -o test4 test4.c
$> ./run.sh ./test4
Hello
```

In case of error, realloc must return NULL. Is "Hello" displayed as in the example?

**If the program reacts unhealthily (segfault or others), the defense stops and select "Crash".**

### Show_alloc_mem Test (test5.c)

```c
#include <stdlib.h>

int main()
{
    malloc(1024);
    malloc(1024*32);
    malloc(1024*1024);
    malloc(1024 * 1024 * 16);
    malloc(1024 * 1024 * 128);
    show_alloc_mem();
    return (0);
}
```

```bash
$> cc -o test5 test5.c -L. -lft_malloc
$> ./test5
```

**Does the display correspond to the topic and TINY/SMALL/LARGE distribution of the project?**

## Bonus

### Concurrent Access

The project manages concurrent thread accesses using pthread library and mutexes.

**Count applicable cases:**
- A mutex prevents multiple threads from entering malloc function simultaneously
- A mutex prevents multiple threads from entering free function simultaneously  
- A mutex prevents multiple threads from entering realloc function simultaneously
- A mutex prevents multiple threads from entering show_alloc_mem function simultaneously

**Rate from 0 (failed) to 5 (excellent)**

### Other Bonuses

**Examples:**
- During free, the project "defragments" free memory by grouping contiguous free blocks
- Malloc has debug environment variables
- Function for hex dump of allocated areas
- Function to display history of memory allocations

**Rate from 0 (failed) to 5 (excellent)**

## Ratings

Don't forget to check the flag corresponding to the defense:

- ✓ OK
- Outstanding project
- Empty work
- Incomplete work
- No author file
- Invalid compilation
- Standard
- Cheat
- Crash
- Forbidden function