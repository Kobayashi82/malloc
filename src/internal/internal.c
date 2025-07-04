/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   internal.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:40:10 by vzurera-          #+#    #+#             */
/*   Updated: 2025/07/04 12:33:43 by vzurera-         ###   ########.fr       */
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
			if (print_log(1)) aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Mutex failed\n");
			if (print_error())	aprintf(2, 0, "Mutex failed\n");
			abort(); return (1);
		}

		return (result);
	}

#pragma endregion

#pragma region "Page Size"

	size_t get_pagesize() {
		static size_t pagesize = 0;

		if (!pagesize) pagesize = (size_t)sysconf(_SC_PAGESIZE);
		if (!pagesize) pagesize = 4096;

		return (pagesize);
	}

#pragma endregion

#pragma region "Fork"

	#pragma region "Prepare"

		int try_lock_timeout(pthread_mutex_t *mtx_ptr, int timeout) {
			for (int i = 0; i < timeout; i++) {
				int ret = mutex(mtx_ptr, MTX_TRYLOCK);
				if (!ret) return (0);

				if (ret != EBUSY) {
					if (print_log(1)) aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] locking mutex in fork\n");
					return (ret);
				}
				
				usleep(1000);
			}

			if (print_log(1)) aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] timeout locking mutex in fork\n");
			return (ETIMEDOUT);
		}

		void prepare_fork() {
			if (print_log(2)) aprintf(g_manager.options.fd_out, 1, "\t\t [SYSTEM] Prepare fork\n");

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
			if (print_log(2)) aprintf(g_manager.options.fd_out, 1, "\t\t [SYSTEM] Parent fork\n");

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
			if (print_log(2)) aprintf(g_manager.options.fd_out, 1, "\t\t [SYSTEM] Child fork\n");

			t_arena *arena = &g_manager.arena;
			while (arena) {
				mutex(&arena->mutex, MTX_UNLOCK);
				arena = arena->next;
			}
			mutex(&g_manager.mutex, MTX_UNLOCK);
		}

	#pragma endregion

#pragma endregion

#pragma region "Alloc"

	void *internal_alloc(size_t size) {
		if (!size) return (NULL);

		size_t total_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
		void *ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (ptr == MAP_FAILED) {
			if (print_log(1))	aprintf(g_manager.options.fd_out, 1, "\t\t  [ERROR] Unable to map memory (internal allocation)\n");
			if (print_error())	aprintf(2, 0, "Memory exhausted\n");
			abort(); return (NULL);
		}

		return (ptr);
	}

#pragma endregion

#pragma region "Free"

	int internal_free(void *ptr, size_t size) {
		if (!ptr || !size) return (0);

		if (munmap(ptr, size)) {
			if (print_log(1)) aprintf(g_manager.options.fd_out, 1, "%p\t  [ERROR] Unable to unmap memory (internal allocation)\n", ptr);
			return (1);
		}

		return (0);
	}

#pragma endregion

#pragma region "Print"

	bool print_log(int mode) {
		if (mode == 1) return (g_manager.options.DEBUG || g_manager.options.LOGGING);
		if (mode == 2) return (g_manager.options.DEBUG == 2 && g_manager.options.LOGGING);
		return (g_manager.options.LOGGING);
	}

	bool print_error() {
		return (!g_manager.options.DEBUG && g_manager.options.LOGGING != 2 && g_manager.options.CHECK_ACTION != 2);
	}

#pragma endregion

#pragma region "Abort"

	int abort_now() {
		if (!g_manager.options.CHECK_ACTION) abort();
		return (1);
	}

#pragma endregion

#pragma region "Initialize"

	void forksafe(void) { pthread_atfork(prepare_fork, parent_fork, child_fork); }

	void ensure_init() {
		static bool initialized;

		if (!initialized) {
			initialized = true;
			mutex(&g_manager.mutex, MTX_INIT);
			mutex(&g_manager.hist_mutex, MTX_INIT);
			get_pagesize();
			options_initialize();
			static pthread_once_t init_once = PTHREAD_ONCE_INIT;
			pthread_once(&init_once, forksafe);
		}
	}

	__attribute__((constructor)) static void malloc_initialize() { ensure_init(); }

#pragma endregion
