/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/29 21:42:58 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/30 22:12:17 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <pthread.h>
	#include <malloc.h>	// para mallopt()
	#include <sys/mman.h>
	#include <errno.h>
	#include <sys/wait.h>

#pragma endregion

#pragma region "Defines"

	#ifndef DEBUG_MODE
		#define DEBUG_MODE 0
	#endif

	#define M_ARENA_MAX					-8				//
	#define M_ARENA_TEST				-7				//
	#define M_PERTURB					-6				//
	#define M_CHECK_ACTION				-5				//
	#define M_MXFAST			 		 1				//
	#define M_FRAG_PERCENT			 	 2				// Si una zona esta mas fragmentada que esto, no usarla (crear nueva si es necesario)
	#define M_MIN_USAGE_PERCENT			 3				// Si una zona esta menos usada que esto, no usarla (pero si todas estan por debajo del threshold, usar la de mayor tamaño)
	#define M_DEBUG						 7				// (DEBUG) Enable debug mode
	#define M_LOGGING					 8				// (DEBUG) Captura backtrace con backtrace() y lo guardas junto con cada allocación.
	#define M_LOGFILE					 9				// (DEBUG) Con diferentes comportamientos según el valor:

	#define THREADS						 4				// 
	#define THREADS_ALLOC				 1				// 
	#define TINY_ALLOC					 16				// 
	#define SMALL_ALLOC					 64				// 
	#define MEDIUM_ALLOC				 570			// 
	#define LARGE_ALLOC					 1024 * 1024	// 

	int aprintf(int fd, char const *format, ...);

#pragma endregion

#pragma region "Tests"

	#pragma region "Thread"

		void *thread_test(void *arg) {
			int thread_num = *(int *)arg;
			char *str;

			// SMALL allocation
			for (int i = 0; i < THREADS_ALLOC; i++) {
				str = malloc(SMALL_ALLOC);
				if (str) {
					str[0] = 'a';
					if (!DEBUG_MODE) aprintf(1, "[MALLOC]\tAllocated (%d bytes) for thread #%d\t\t(%p)\n", SMALL_ALLOC, thread_num, str);
					free(str);
				} else {
					if (!DEBUG_MODE) aprintf(1, "[ERROR]\tMalloc failed for thread #%d\n", thread_num);
				}
			}

			// LARGE allocation 
			str = malloc(LARGE_ALLOC);
			if (str) {
				str[0] = 'a';
				if (!DEBUG_MODE) aprintf(1, "[MALLOC]\tAllocated (%d bytes) for thread #%d\t\t(%p)\n", LARGE_ALLOC, thread_num, str);
				free(str);
			} else {
				if (!DEBUG_MODE) aprintf(1, "[ERROR]\tMalloc failed for thread #%d\n", thread_num);
			}

			return (NULL);
		}

	#pragma endregion

	#pragma region "Realloc"

		void realloc_test() {
			aprintf(1, "\n=== Realloc ===\n\n");

			// SMALL allocation
			char *ptr = malloc(TINY_ALLOC);
			if (ptr) {
				strcpy(ptr, "TINY");
				if (!DEBUG_MODE) aprintf(1, "[MALLOC]\tAllocated (%d bytes)\t\t\t\t(%p)\n", TINY_ALLOC, ptr);

				// SMALL re-allocation
				ptr = realloc(ptr, SMALL_ALLOC);
				if (ptr) {
					strcpy(ptr, "SMALL");
					if (!DEBUG_MODE) aprintf(1, "[REALLOC]\tExtended (%d bytes)\t\t\t\t(%p)\n", SMALL_ALLOC, ptr);
				} else {
					if (!DEBUG_MODE) aprintf(1, "[ERROR]\tRealloc failed\n");
				}
				free(ptr);
			} else {
				if (!DEBUG_MODE) aprintf(1, "[ERROR]\tRealloc failed\n");
			}
		}

	#pragma endregion

	#pragma region "Heap"

		void heap_test() {
			aprintf(1, "\n=== Heap ===\n\n");

			// TINY allocation
			char *small = (char *)malloc(TINY_ALLOC);
			if (small) {
				strcpy(small, "TINY");
				if (!DEBUG_MODE) aprintf(1, "[MALLOC]\tAllocated (%d bytes)\t\t\t\t(%p)\n", TINY_ALLOC, small);
			} else if (!DEBUG_MODE) aprintf(1, "[ERROR]\tMalloc failed\n");
			free(small);

			// MEDIUM allocation
			char *medium = (char *)malloc(MEDIUM_ALLOC);
			if (medium) {
				strcpy(medium, "MEDIUM");
				if (!DEBUG_MODE) aprintf(1, "[MALLOC]\tAllocated (%d bytes)\t\t\t\t(%p)\n", MEDIUM_ALLOC, medium);
			} else if (!DEBUG_MODE) aprintf(1, "[ERROR]\tMalloc failed\n");
			free(medium);

			// LARGE allocation
			char *large = (char *)malloc(LARGE_ALLOC);
			if (large) {
				strcpy(large, "LARGE");
				if (!DEBUG_MODE) aprintf(1, "[MALLOC]\tAllocated (%d bytes)\t\t\t(%p)\n", LARGE_ALLOC, large);
			} else if (!DEBUG_MODE) aprintf(1, "[ERROR]\tMalloc failed\n");
			free(large);
		}

	#pragma endregion

#pragma endregion

#pragma region "Main"

	int main() {
		mallopt(M_DEBUG, DEBUG_MODE);
		mallopt(M_ARENA_TEST, 20);
		mallopt(M_ARENA_MAX, 0);

		heap_test();
		realloc_test();

		aprintf(1, "\n=== Threads ===\n\n");

		int i, n_threads = THREADS;
		int thread_args[n_threads];
		pthread_t threads[n_threads];

		for (i = 0; i < n_threads; i++) {
			thread_args[i] = i + 1;
			if (pthread_create(&threads[i], NULL, thread_test, &thread_args[i]) != 0) {
				aprintf(1, "[ERROR]\tThread creation failed for thread %d\n", i + 1);
				n_threads = i; break;
			}
		}

		int pid = fork();
		if (pid == -1) {
			aprintf(1, "[ERROR]\tFork failed\n");
			return (1);
		} else if (pid == 0) {
			heap_test();
			return (0);
		}

		for (i = 0; i < n_threads; i++) {
			if (pthread_join(threads[i], NULL) != 0) {
				aprintf(1, "[ERROR]\tThread join failed for thread %d\n", i + 1);
			}
		}

		waitpid(pid, NULL, 0);
		return (0);
	}

#pragma endregion

#pragma region "Information"

	// NOTE: Con THREADS = 152 se produce un segfault

	// A partir del 5º hilo parece ser que pthread_create deja de usar el pool de threads y empieza a usar malloc de la glibc.
	// La llamada la hace internamente y no la puedo interceptar con mi malloc.
	// Lo curioso es que en pthread_join se llama a free y esta si se intercepta. Lo cual da lugar a que mi free recibe un puntero
	// por hilo (a partir del 5º) que no puede liberar porque no pertenece a mi malloc.
	// La solucion es que si mi free recibe un puntero que no pertenece a mi malloc, llama a free de la glibc para que lo procese.

#pragma endregion
