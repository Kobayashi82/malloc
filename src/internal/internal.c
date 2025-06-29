/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   internal.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:40:10 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/29 13:10:49 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "internal.h"

#pragma endregion

#pragma region "Variables"

	t_manager			g_manager;
	__thread t_arena	*tcache;

#pragma endregion

#pragma region "Mutex"

	int mutex(pthread_mutex_t *ptr_mutex, int action) {
		int result = 0;

		switch (action) {
			case MTX_INIT:		result = pthread_mutex_init(ptr_mutex, NULL);	break;
			case MTX_LOCK:		result = pthread_mutex_lock(ptr_mutex);			break;
			case MTX_UNLOCK:	result = pthread_mutex_unlock(ptr_mutex);		break;
			case MTX_DESTROY:	return (pthread_mutex_destroy(ptr_mutex));
			case MTX_TRYLOCK:	return (pthread_mutex_trylock(ptr_mutex));
		}

		if (result) {
			if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Mutex failed\n");
			abort();
		}

		return (result);
	}

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

#pragma region "Check Digit"

	int check_digit(void *ptr1, void *ptr2) {
		if (!ptr1 || !ptr2) return (0);

		uintptr_t val1 = (uintptr_t)ptr1;
		uintptr_t val2 = (uintptr_t)ptr2;

		while (val1 >= 0x10) val1 /= 0x10;
		while (val2 >= 0x10) val2 /= 0x10;

		return ((val1 & 0xF) == (val2 & 0xF));
	}

#pragma endregion

#pragma region "Fork"

	#pragma region "Prepare"

		int try_lock_timeout(pthread_mutex_t *mtx_ptr, int timeout) {
			for (int i = 0; i < timeout; i++) {
				int ret = mutex(mtx_ptr, MTX_TRYLOCK);
				if (!ret) return (0);

				if (ret != EBUSY) {
					if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] locking mutex in fork\n");
					return (ret);
				}
				
				// Wait 1ms before next try
				#ifdef _WIN32
					Sleep(1);
				#else
					usleep(1000);
				#endif
			}

			if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] timeout locking mutex in fork\n");
			return (ETIMEDOUT);
		}

		void prepare_fork() {
			if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t [SYSTEM] Prepare fork\n");

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

		void parent_fork() {
			if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t [SYSTEM] Parent fork\n");

			t_arena *arena = &g_manager.arena;
			while (arena) {
				mutex(&arena->mutex, MTX_UNLOCK);
				arena = arena->next;
			}
			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

	#pragma endregion

	#pragma region "Child"

		void child_fork() {
			if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t [SYSTEM] Child fork\n");

			t_arena *arena = &g_manager.arena;
			while (arena) {
				mutex(&arena->mutex, MTX_UNLOCK);
				arena = arena->next;
			}
			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

	#pragma endregion

	#pragma region "Initialize"

		void do_init(void) {
			pthread_atfork(prepare_fork, parent_fork, child_fork);
		}

		void forksafe_init() {
			static pthread_once_t init_once = PTHREAD_ONCE_INIT;
			pthread_once(&init_once, do_init);
		}

	#pragma endregion

#pragma endregion

#pragma region "Alloc"

	void *internal_alloc(size_t size) {
		if (!size) return (NULL);

		size_t total_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
		void *ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (ptr == MAP_FAILED) {
			if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "\t\t  [ERROR] Unable to map memory (internal allocation)\n");
			abort(); return (NULL);
		}

		return (ptr);
	}

#pragma endregion

#pragma region "Free"

	int internal_free(void *ptr, size_t size) {
		if (!ptr || !size) return (0);

		if (munmap(ptr, size)) {
			if (g_manager.options.DEBUG) aprintf(g_manager.options.fd_out, "%p\t  [ERROR] Unable to unmap memory (internal allocation)\n", ptr);
			return (1);
		}

		return (0);
	}

#pragma endregion

#pragma region "Abort"

	int abort_now() {
		// t_arena *arena = tcache;
		// t_heap_header *heap_header = arena->heap_header;

		// while (heap_header) {
		// 	t_heap *heap = (t_heap *)((char *)heap_header + ALIGN(sizeof(t_heap_header)));

		// 	for (int i = 0; i < heap_header->used; ++i) {
		// 		char *type;
		// 		if (heap->type == TINY) type = "Tiny";
		// 		if (heap->type == SMALL) type = "Small";
		// 		if (heap->type == LARGE) type = "Large";
		// 		aprintf(g_manager.options.fd_out, "heap: %p\nptr: %p\ntype: %s\nActive: %s\n\n", heap, heap->ptr, type, heap->active == true ? "True" : "False");

		// 		heap = (t_heap *)((char *)heap + ALIGN(sizeof(t_heap)));
		// 	}

		// 	heap_header = heap_header->next;
		// }

		if (!g_manager.options.CHECK_ACTION) abort();
		return (1);
	}

#pragma endregion

#pragma region "Initialize"

	void ensure_init() {
		static bool initialized;

		if (!initialized) {
			initialized = true;
			mutex(&g_manager.mutex, MTX_INIT);
			get_pagesize();
			options_initialize();
			forksafe_init();
		}
	}

	__attribute__((constructor)) static void malloc_initialize() { ensure_init(); }

#pragma endregion
