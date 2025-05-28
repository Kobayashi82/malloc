/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 23:58:18 by vzurera-          #+#    #+#             */
/*   Updated: 2025/05/28 22:32:41 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"
	#include "utils.h"

#pragma endregion

#pragma region "Variables"

	t_manager			g_manager;
	__thread t_arena	*thread_arena;

	static int	arena_initialize(t_arena *arena);
	static void	arena_terminate();

#pragma endregion

#pragma region "Constructor"

	__attribute__((constructor)) static void malloc_initialize() {
		options_initialize();
	}

	__attribute__((destructor)) static void malloc_terminate() {
		arena_terminate();
	}

#pragma endregion

#pragma region "Internal"

	#pragma region "Alloc"

		static void *internal_alloc(size_t size) {
			void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			return (ptr == MAP_FAILED ? NULL : ptr);
		}

	#pragma endregion

	#pragma region "Free"

		static void internal_free(void *ptr, size_t size) {
			if (ptr && size > 0) munmap(ptr, size);
		}

	#pragma endregion

	#pragma region "Mutex"

		int mutex(mtx_t *ptr_mutex, e_mutex action) {
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

#pragma endregion

#pragma region "Arena"

	#pragma region "Initialize"

		int arena_initialize(t_arena *arena) {
			arena->id = g_manager.arena_count + 1;
			arena->used = 0;
			// bins
			arena->heap[0] = NULL;
			arena->heap[1] = NULL;
			arena->heap[2] = NULL;
			arena->next = NULL;
			mutex(&arena->mutex, MTX_INIT);
			g_manager.arena_count++;

			return (0);
		}

	#pragma endregion

	#pragma region "Terminate"

		void arena_terminate() {
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
		}

	#pragma endregion

	#pragma region "Create"

		#pragma region "Get CPUs"

			int get_CPUs() {
				int CPUs = 1;

				#ifdef _WIN32
					SYSTEM_INFO sysinfo;
					GetSystemInfo(&sysinfo);
					CPUs = sysinfo.dwNumberOfProcessors;
				#else
					CPUs = sysconf(_SC_NPROCESSORS_ONLN);
				#endif

				return (CPUs <= 0 ? 1 : CPUs);
			}

		#pragma endregion

		#pragma region "Can Create"

			static int arena_can_create() {
				if (!g_manager.options.ARENA_MAX) {
					if (g_manager.arena_count >= g_manager.options.ARENA_TEST) {
						g_manager.options.ARENA_MAX = get_CPUs() * 2;
						return (g_manager.arena_count < g_manager.options.ARENA_MAX);
					} else return (1);
				}
				return (g_manager.arena_count < g_manager.options.ARENA_MAX);
			}

		#pragma endregion

		#pragma region "Create"

			t_arena *arena_create() {
				t_arena *new_arena;

				if (!arena_can_create()) return (NULL);

				new_arena = internal_alloc(sizeof(t_arena));
				if (!new_arena) return (NULL);

				if (arena_initialize(new_arena)) {
					internal_free(new_arena, sizeof(t_arena));
					return (NULL);
				}

				t_arena *current = &g_manager.arena;
				while (current->next) current = current->next;
				current->next = new_arena;
				g_manager.arena_count++;
				new_arena->id = g_manager.arena_count;

				return (new_arena);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Reuse"

		t_arena *arena_reuse() {
			t_arena *current, *best_arena = NULL;

			current = &g_manager.arena;
			while (current) {
				if (!mutex(&current->mutex, MTX_TRYLOCK)) {
					best_arena = current;
					mutex(&current->mutex, MTX_UNLOCK);
					break;
				} else {
					ft_aprintf(1, "WTF\n");
				}
				current = current->next;
			}

			return (best_arena);
		}

	#pragma endregion

	#pragma region "Get"

		t_arena *arena_get() {
			t_arena *arena = NULL;

			mutex(&g_manager.mutex, MTX_LOCK);

				if (!g_manager.initialized) {
					g_manager.initialized = true;
					g_manager.arena_count = 0;
					if (arena_initialize(&g_manager.arena)) abort();
					arena = &g_manager.arena;
				}
				if (!arena) arena = arena_reuse();
				if (!arena) arena = arena_create();
				if (!arena) arena = &g_manager.arena;

			mutex(&g_manager.mutex, MTX_UNLOCK);

			return (arena);
		}

	#pragma endregion

#pragma endregion