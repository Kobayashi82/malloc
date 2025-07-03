#include "malloc.h"
#include <stdlib.h>
#include <string.h>

int main()
{
	malloc(1024);
	malloc(1024 * 32);
	malloc(1024 * 1024);
	malloc(1024 * 1024 * 16);
	malloc(1024 * 1024 * 128);
	show_alloc_mem();
	char *ptr = malloc(30);
	strcpy(ptr, "Hello World!\n");
	show_alloc_mem_ex(ptr, 0, 0);
	return (0);
}
