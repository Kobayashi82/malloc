/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   internal.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:40:10 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/26 23:56:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"

#pragma endregion

#pragma region "Variables"

	t_manager			g_manager;
	__thread t_arena	*tcache;

#pragma endregion

#pragma region "Mutex"

	int mutex(mtx_t *ptr_mutex, int action) {
		int result = 0;

		switch (action) {
			case MTX_INIT:		result = pthread_mutex_init(ptr_mutex, NULL);	break;
			case MTX_LOCK:		result = pthread_mutex_lock(ptr_mutex);			break;
			case MTX_UNLOCK:	result = pthread_mutex_unlock(ptr_mutex);		break;
			case MTX_DESTROY:	return (pthread_mutex_destroy(ptr_mutex));
			case MTX_TRYLOCK:	return (pthread_mutex_trylock(ptr_mutex));
		}

		if (result) abort();
		return (result);
	}

#pragma endregion

#pragma region "Fork"

	#pragma region "Prepare"

		static int try_lock_timeout(mtx_t *mtx_ptr, int timeout) {
			for (int i = 0; i < timeout; i++) {
				int ret = mutex(mtx_ptr, MTX_TRYLOCK);
				if (!ret) return (0);

				if (ret != EBUSY) {
					if (g_manager.options.DEBUG) aprintf(2, "\t\t  [ERROR] locking mutex in fork\n");
					return (ret);
				}
				
				// Wait 1ms before next try
				#ifdef _WIN32
					Sleep(1);
				#else
					usleep(1000);
				#endif
			}

			if (g_manager.options.DEBUG) aprintf(2, "\t\t  [ERROR] timeout locking mutex in fork\n");
			return (ETIMEDOUT);
		}

		static void prepare_fork() {
			if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Prepare fork\n");

			int ret = try_lock_timeout(&g_manager.mutex, 1000);
			if (ret) return;

			t_arena *arena = &g_manager.arena;
			while (arena) {
				ret = try_lock_timeout(&arena->mutex, 1000);
				if (ret) {
					t_arena *arena_ptr = arena;
					arena = &g_manager.arena;
					while (arena && arena != arena_ptr) {
						mutex(&arena->mutex, MTX_UNLOCK);
						arena = arena->next;
					}
					mutex(&g_manager.mutex, MTX_UNLOCK);
					return;
				}
				arena = arena->next;
			}
		}

	#pragma endregion

	#pragma region "Parent"

		static void parent_fork() {
			if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Parent fork\n");

			t_arena *arena = &g_manager.arena;
			while (arena) {
				mutex(&arena->mutex, MTX_UNLOCK);
				arena = arena->next;
			}
			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

	#pragma endregion

	#pragma region "Child"

		static void child_fork() {
			if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Child fork\n");

			t_arena *arena = &g_manager.arena;
			while (arena) {
				mutex(&arena->mutex, MTX_UNLOCK);
				arena = arena->next;
			}
			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

	#pragma endregion

#pragma endregion

#pragma region "Page Size"

	size_t get_pagesize() {
		static size_t pagesize = 0;

		if (!pagesize) {
			#ifdef _WIN32
				SYSTEM_INFO info;
				GetSystemInfo(&info);
				pagesize = (size_t)info.dwPageSize;
			#else
				pagesize = (size_t)sysconf(_SC_PAGESIZE);
			#endif
		}

		if (!pagesize) pagesize = 4096;
		return (pagesize);
	}

#pragma endregion

#pragma region "Internal"

	#pragma region "Alloc"

		void *internal_alloc(size_t size) {
			if (!size) return (NULL);

			size_t total_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
			void *ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			if (ptr == MAP_FAILED) {
				if (g_manager.options.DEBUG) aprintf(2, "\t\t  [ERROR] Unable to map memory (internal allocation)\n");
				abort();
			}

			return (ptr);
		}

	#pragma endregion

	#pragma region "Free"

		int internal_free(void *ptr, size_t size) {
			if (!ptr || !size) return (0);

			if (munmap(ptr, size)) {
				if (g_manager.options.DEBUG) aprintf(2, "%p\t  [ERROR] Unable to unmap memory (internal allocation)\n", ptr);
				return (1);
			}

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Terminate"

	void arena_terminate2() {
		t_arena *current, *next;

		mutex(&g_manager.mutex, MTX_LOCK);

			mutex(&g_manager.arena.mutex, MTX_DESTROY);
			current = g_manager.arena.next;
			while (current) {
				mutex(&current->mutex, MTX_DESTROY);

				// liberar zonas

				next = current->next;
				internal_free(current, sizeof(t_arena));
				current = next;
			}

		mutex(&g_manager.mutex, MTX_UNLOCK);
		mutex(&g_manager.mutex, MTX_DESTROY);

		if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Library finalized\n");
	}

	#include <fcntl.h>

	void arena_terminate() {
		t_arena	*arena, *next;

		// if debug, print memoria status (free, not free, heaps, etc.)

		mutex(&g_manager.mutex, MTX_LOCK);

			arena = &g_manager.arena;
			while (arena) {
				mutex(&arena->mutex, MTX_LOCK);

					t_heap_header *heap_header = arena->heap_header;
					while (heap_header) {
						t_heap *heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));

						for (int i = 0; i < heap_header->used; i++) {
							if (heap->active) heap_destroy(arena, heap->ptr, heap->type, heap->size);
							heap++;
						}

						heap_header = heap_header->next;
					}

					// clean info pages

					next = arena->next;
				
				mutex(&arena->mutex, MTX_UNLOCK);
				mutex(&arena->mutex, MTX_DESTROY);

				// if (arena != &g_manager.arena) internal_free(arena, sizeof(t_arena));
				arena = next;
			}

		mutex(&g_manager.mutex, MTX_UNLOCK);
		mutex(&g_manager.mutex, MTX_DESTROY);

		// No funciona write en el destructor
		//
		// if (g_manager.options.DEBUG) {
		// 	int fd = open("/tmp/malloc_debug.log", O_CREAT | O_WRONLY | O_APPEND, 0644);
		// 	if (fd != -1) {
		// 		aprintf(fd, "\t\t [SYSTEM] Library finalized\n");
		// 		close(fd);
		// 	}
		// }

	}

#pragma endregion

#pragma region "Constructor"

	static pthread_once_t init_once = PTHREAD_ONCE_INIT;

	static void do_init(void) {
		mutex(&g_manager.mutex, MTX_INIT);
		options_initialize();
		pthread_atfork(prepare_fork, parent_fork, child_fork);
		if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Library initialized\n");
	}

	void ensure_init() {
		pthread_once(&init_once, do_init);
	}

	__attribute__((destructor)) static void malloc_terminate() {
		arena_terminate();
	}

#pragma endregion
