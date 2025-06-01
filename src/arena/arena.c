/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arena.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 23:58:18 by vzurera-          #+#    #+#             */
/*   Updated: 2025/06/01 15:04:40 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma region "Includes"

	#include "arena.h"
	#include "malloc.h"
	#include "utils.h"

#pragma endregion

#pragma region "Variables"

	t_manager			g_manager;
	__thread t_arena	*tcache;

	static int	arena_initialize(t_arena *arena);
	static void	arena_terminate();
	static void	prepare_fork();
	static void	parent_fork();
	static void	child_fork();

#pragma endregion

#pragma region "Constructor"

	__attribute__((constructor)) static void malloc_initialize() {
		mutex(&g_manager.mutex, MTX_INIT);
		realfree(NULL);
		options_initialize();
		pthread_atfork(prepare_fork, parent_fork, child_fork);
	}

	__attribute__((destructor)) static void malloc_terminate() {
		arena_terminate();
		mutex(&g_manager.mutex, MTX_DESTROY);
	}

#pragma endregion

#pragma region "Internal"


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

	#pragma region "Fork"

		#pragma region "Prepare"

			static int try_lock_timeout(mtx_t *mtx_ptr, int timeout) {
				for (int i = 0; i < timeout; i++) {
					int ret = mutex(mtx_ptr, MTX_TRYLOCK);
					if (!ret) return (0);

					if (ret != EBUSY) {
						if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] locking mutex in fork\n");
						return (ret);
					}
					
					// Wait 1ms before next try
					#ifdef _WIN32
						Sleep(1);
					#else
						usleep(1000);
					#endif
				}

				if (g_manager.options.DEBUG) aprintf(1, "\t\t  [ERROR] timeout locking mutex in fork\n");
				return (ETIMEDOUT);
			}

			static void prepare_fork() {
				if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Prepare fork\n");
				
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
				if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Parent fork\n");

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
				if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Child fork\n");

				t_arena *arena = &g_manager.arena;
				while (arena) {
					mutex(&arena->mutex, MTX_UNLOCK);
					arena = arena->next;
				}
				mutex(&g_manager.mutex, MTX_UNLOCK);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region "Arena"

	#pragma region "Initialize"

		int arena_initialize(t_arena *arena) {
			arena->id = g_manager.arena_count + 1;
			// bins
			arena->tiny = NULL;
			arena->small = NULL;
			arena->large = NULL;
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
						g_manager.options.ARENA_MAX = ft_min(get_CPUs() * 2, ARENAS_MAX);
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

				if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Arena #%d created\n", new_arena->id);

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
					// return (NULL);	// Force arena creation
					break;
				} else {
					if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Arena #%d locked\n", current->id);
				}
				current = current->next;
			}

			return (best_arena);
		}

	#pragma endregion

	#pragma region "Get"

		t_arena *arena_get() {
			static bool initialized;
			t_arena *arena = NULL;

			mutex(&g_manager.mutex, MTX_LOCK);

				if (!initialized) {
					initialized = true;
					if (arena_initialize(&g_manager.arena)) abort();
					arena = &g_manager.arena;
					if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Arena #%d created\n", arena->id);
				}
				if (!arena) arena = arena_reuse();
				if (!arena) arena = arena_create();
				if (!arena) arena = &g_manager.arena;

				if (g_manager.options.DEBUG) aprintf(1, "\t\t [SYSTEM] Arena #%d assigned\n", arena->id);

			mutex(&g_manager.mutex, MTX_UNLOCK);

			return (arena);
		}

	#pragma endregion

#pragma endregion
