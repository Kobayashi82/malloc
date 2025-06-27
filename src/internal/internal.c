/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   internal.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:40:10 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/27 14:59:51 by vzurera-         ###   ########.fr       */
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

#pragma region "Constructor"

	void ensure_init() {
		static int initialized = 0;

		if (!initialized) {
			initialized = 1;
			mutex(&g_manager.mutex, MTX_INIT);
			get_pagesize();
			options_initialize();
			if (g_manager.options.DEBUG) aprintf(2, "\t\t [SYSTEM] Arena #%d created\n", g_manager.arena.id);
		}
	}

	__attribute__((constructor)) static void malloc_initialize() { ensure_init(); }

#pragma endregion
