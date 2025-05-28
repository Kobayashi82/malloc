#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>	// para mallopt()

// alineacion
	// #define ALIGNMENT 16
	// #define ALIGN(size) (((size) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
// arenas
// magic number
// zonas
// bitmap
// freelist
// tcache
// free: double free
// free: invalid pointer
// fusion de bloques
// liberacion de zonas
// lazy coalescing (or not)
// best fit vs first fit

#ifndef DEBUG_MODE
	#define DEBUG_MODE 0
#endif

void *thread_test(void *arg) {
	int thread_num = *(int *)arg;
	char *str;
	int i;

	// Asignación y liberación
	for (i = 0; i < 3; i++) {
		str = (char *)malloc(64);
		if (str) {
			sprintf(str, "Asignación #%d", i);
			printf("Hilo %d: %s\t\t(%p)\n", thread_num, str, str);
			free(str);
		} else {
			printf("Hilo %d: Malloc failed\n", thread_num);
		}
	}

	// Asignación LARGE
	str = malloc(1024 * 1024);
	if (str) {
		sprintf(str, "Asignación #%d", i);
		printf("Hilo %d: %s\t\t(%p)\n", thread_num, str, str);
		free(str);
	} else {
		printf("Hilo %d: Malloc failed\n", thread_num);
	}

	return (NULL);
}

void test_realloc() {
	printf("\n=== Realloc ===\n\n");

	int initial = 8, extended = 128;
	char *ptr = malloc(initial);
	if (ptr) {
		strcpy(ptr, "inicial");
		printf("Asignación %s    (%d bytes)\t(%p)\n", ptr, initial, ptr);

		ptr = realloc(ptr, extended);
		if (ptr) {
			strcpy(ptr, "ampliada");
			printf("Asignación %s (%d bytes)\t(%p)\n", ptr, extended, ptr);
		} else {
			printf("Realloc failed\n");
		}
		free(ptr);
	} else {
		printf("Realloc failed\n");
	}
}

void test_zones() {
	printf("\n=== Zones ===\n\n");

	// Asignación TINY
	char *small = (char *)malloc(16);
	if (small) {
		strcpy(small, "TINY");
		printf("Asignación %s\t\t\t(%p)\n", small, small);
	}

	// Asignación SMALL
	char *medium = (char *)malloc(570);
	if (medium) {
		strcpy(medium, "SMALL");
		printf("Asignación %s\t\t(%p)\n", medium, medium);
	}

	// Asignación LARGE
	char *large = (char *)malloc(10240);
	if (large) {
		strcpy(large, "LARGE");
		printf("Asignación %s\t\t(%p)\n", large, large);
	}

	free(small);
	free(medium);
	free(large);
}

int main() {
	mallopt(7, DEBUG_MODE);
	
	test_zones();
	test_realloc();

	printf("\n=== Threads ===\n\n");
	int i, n_threads = 3;
	int thread_args[n_threads];
	pthread_t threads[n_threads];
	
	for (i = 0; i < n_threads; i++) {
		thread_args[i] = i + 1;
		if (pthread_create(&threads[i], NULL, thread_test, &thread_args[i]) != 0) {
			printf("Thread creation failed\n");
			n_threads = i; break;
		}
	}
	for (i = 0; i < n_threads; i++) pthread_join(threads[i], NULL);

	printf("\n");
	return (0);
}